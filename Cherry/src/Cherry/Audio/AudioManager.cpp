#include "CHpch.h"
#include "AudioManager.h"
#include "Cherry/Scene/Components.h"
#include <cmath>

// Platform-specific includes
#ifdef CH_PLATFORM_WINDOWS
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
// Undefine Windows macros after mmsystem.h
#ifdef PlaySoundW
#undef PlaySoundW
#endif
#ifdef PlaySoundA
#undef PlaySoundA
#endif
#ifdef PlaySound
#undef PlaySound
#endif
#elif defined(CH_PLATFORM_LINUX)
#include <alsa/asoundlib.h>
#endif

namespace Cherry {

    bool AudioManager::Initialize() {
        if (m_IsInitialized) {
            CH_CORE_WARN("AudioManager already initialized");
            return true;
        }

        CH_CORE_INFO("Initializing AudioManager...");

        // Initialize audio device
        if (!InitializeAudioDevice()) {
            CH_CORE_ERROR("Failed to initialize audio device");
            return false;
        }

        // Create default mixer channels
        CreateMixerChannel("Master");
        CreateMixerChannel("Music");
        CreateMixerChannel("SFX");
        CreateMixerChannel("Voice");
        CreateMixerChannel("UI");

        // Initialize spectrum analysis
        m_SpectrumData.resize(64, 0.0f);

        // Start audio thread
        m_AudioThreadRunning = true;
        m_AudioThread = std::thread([this]() {
            while (m_AudioThreadRunning) {
                {
                    std::lock_guard<std::mutex> lock(m_AudioMutex);
                    CleanupFinishedSources();
                    UpdateMixerChannels();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
            }
            });

        m_IsInitialized = true;
        CH_CORE_INFO("AudioManager initialized successfully");
        return true;
    }

    void AudioManager::Shutdown() {
        if (!m_IsInitialized) return;

        CH_CORE_INFO("Shutting down AudioManager...");

        // Stop audio thread
        m_AudioThreadRunning = false;
        if (m_AudioThread.joinable()) {
            m_AudioThread.join();
        }

        // Stop all active sources
        {
            std::lock_guard<std::mutex> lock(m_AudioMutex);
            for (auto& [handle, source] : m_ActiveSources) {
                StopSourceInternal(*source);
                DestroySourceInternal(*source);
            }
            m_ActiveSources.clear();
        }

        // Unload all clips
        for (auto& [path, clip] : m_AudioClips) {
            UnloadClipInternal(*clip);
        }
        m_AudioClips.clear();

        // Shutdown audio device
        ShutdownAudioDevice();

        m_IsInitialized = false;
        CH_CORE_INFO("AudioManager shut down successfully");
    }

    bool AudioManager::LoadAudioClip(const std::string& path, bool preload) {
        if (m_AudioClips.find(path) != m_AudioClips.end()) {
            return true; // Already loaded
        }

        auto clip = std::make_unique<AudioClip>(path);
        clip->Format = AudioLoader::GetFormatFromExtension(path);

        if (preload) {
            if (!LoadClipInternal(*clip)) {
                CH_CORE_ERROR("Failed to load audio clip: {}", path);
                return false;
            }
        }

        m_AudioClips[path] = std::move(clip);
        CH_CORE_TRACE("Audio clip loaded: {}", path);
        return true;
    }

    AudioHandle AudioManager::PlaySound(const std::string& clipPath, float volume, float pitch, bool loop) {
        return PlaySoundOnChannel(clipPath, "SFX", volume, pitch, loop);
    }

    AudioHandle AudioManager::PlaySoundOnChannel(const std::string& clipPath, const std::string& channel,
        float volume, float pitch, bool loop) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);

        // Load clip if not already loaded
        if (!LoadAudioClip(clipPath)) {
            return INVALID_AUDIO_HANDLE;
        }

        const AudioClip* clip = GetClip(clipPath);
        if (!clip) {
            CH_CORE_ERROR("Failed to get audio clip: {}", clipPath);
            return INVALID_AUDIO_HANDLE;
        }

        // Check if we have reached the maximum number of active sources
        if (m_ActiveSources.size() >= m_MaxActiveSources) {
            CH_CORE_WARN("Maximum number of audio sources reached");
            return INVALID_AUDIO_HANDLE;
        }

        // Create audio source
        AudioHandle handle = CreateSourceInternal(clip, false);
        if (handle == INVALID_AUDIO_HANDLE) {
            return INVALID_AUDIO_HANDLE;
        }

        auto source = std::make_unique<AudioSource>(handle, clipPath);
        source->Volume = volume;
        source->Pitch = pitch;
        source->IsLooping = loop;
        source->State = AudioState::Playing;

        // Add to mixer channel
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->Sources.push_back(handle);
        }

        // Update source properties and play
        UpdateSourceProperties(*source);
        PlaySourceInternal(*source);

        m_ActiveSources[handle] = std::move(source);
        return handle;
    }

    AudioHandle AudioManager::PlaySound3D(const std::string& clipPath, const glm::vec3& position,
        float volume, float pitch, bool loop) {
        return PlaySound3DOnChannel(clipPath, "SFX", position, volume, pitch, loop);
    }

    AudioHandle AudioManager::PlaySound3DOnChannel(const std::string& clipPath, const std::string& channel,
        const glm::vec3& position, float volume,
        float pitch, bool loop) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);

        // Load clip if not already loaded
        if (!LoadAudioClip(clipPath)) {
            return INVALID_AUDIO_HANDLE;
        }

        const AudioClip* clip = GetClip(clipPath);
        if (!clip) {
            return INVALID_AUDIO_HANDLE;
        }

        // Check if we have reached the maximum number of active sources
        if (m_ActiveSources.size() >= m_MaxActiveSources) {
            return INVALID_AUDIO_HANDLE;
        }

        // Create 3D audio source
        AudioHandle handle = CreateSourceInternal(clip, true);
        if (handle == INVALID_AUDIO_HANDLE) {
            return INVALID_AUDIO_HANDLE;
        }

        auto source = std::make_unique<AudioSource>(handle, clipPath);
        source->Volume = volume;
        source->Pitch = pitch;
        source->IsLooping = loop;
        source->Is3D = true;
        source->Settings3D.Position = position;
        source->State = AudioState::Playing;

        // Add to mixer channel
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->Sources.push_back(handle);
        }

        // Update source properties and play
        UpdateSourceProperties(*source);
        PlaySourceInternal(*source);

        m_ActiveSources[handle] = std::move(source);
        return handle;
    }

    void AudioManager::PlayMusic(const std::string& musicPath, float fadeInTime, bool loop) {
        // Stop current music if playing
        if (m_CurrentMusicHandle != INVALID_AUDIO_HANDLE) {
            StopMusic(fadeInTime > 0 ? fadeInTime : 0.0f);
        }

        m_CurrentMusicHandle = PlaySoundOnChannel(musicPath, "Music", m_MusicVolume, 1.0f, loop);
        m_CurrentMusicPath = musicPath;

        if (fadeInTime > 0.0f && m_CurrentMusicHandle != INVALID_AUDIO_HANDLE) {
            auto* source = GetSource(m_CurrentMusicHandle);
            if (source) {
                source->Volume = 0.0f;
                source->FadeTarget = m_MusicVolume;
                source->FadeSpeed = m_MusicVolume / fadeInTime;
                source->State = AudioState::Fading;
            }
        }
    }

    void AudioManager::Update(float deltaTime) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);

        UpdateFading(deltaTime);
        Update3DAudio();

        // Update streaming sources
        for (auto& [handle, streamingSource] : m_StreamingSources) {
            UpdateAudioStream(handle);
        }

        // Update spectrum analysis if enabled
        if (m_SpectrumAnalysisEnabled) {
            // This would typically involve getting data from the audio device
            // Implementation depends on the audio library being used
        }
    }

    void AudioManager::UpdateFading(float deltaTime) {
        for (auto& [handle, source] : m_ActiveSources) {
            if (source->State == AudioState::Fading) {
                float volumeDelta = source->FadeSpeed * deltaTime;

                if (source->Volume < source->FadeTarget) {
                    source->Volume = std::min(source->Volume + volumeDelta, source->FadeTarget);
                }
                else {
                    source->Volume = std::max(source->Volume - volumeDelta, source->FadeTarget);
                }

                if (std::abs(source->Volume - source->FadeTarget) < 0.01f) {
                    source->Volume = source->FadeTarget;
                    if (source->FadeTarget == 0.0f) {
                        source->State = AudioState::Stopped;
                        StopSourceInternal(*source);
                    }
                    else {
                        source->State = AudioState::Playing;
                    }
                }

                UpdateSourceProperties(*source);
            }
        }
    }

    void AudioManager::Update3DAudio() {
        for (auto& [handle, source] : m_ActiveSources) {
            if (source->Is3D && source->State == AudioState::Playing) {
                // Calculate distance attenuation
                float distance = glm::length(source->Settings3D.Position - m_Listener.Position);
                float attenuation = 1.0f;

                if (distance > source->Settings3D.MinDistance) {
                    attenuation = source->Settings3D.MinDistance /
                        (source->Settings3D.MinDistance +
                            source->Settings3D.RolloffFactor * (distance - source->Settings3D.MinDistance));
                }

                if (distance > source->Settings3D.MaxDistance) {
                    attenuation = 0.0f;
                }

                // Apply 3D calculations to source volume
                source->Volume = std::min(source->Volume, attenuation);

                // Calculate doppler effect if enabled
                if (source->Settings3D.EnableDoppler && distance > 0.001f) {
                    glm::vec3 direction = glm::normalize(source->Settings3D.Position - m_Listener.Position);
                    glm::vec3 relativeVelocity = source->Settings3D.Velocity - m_Listener.Velocity;
                    float velocityTowards = glm::dot(direction, relativeVelocity);
                    
                    // Prevent division by zero and extreme doppler effects
                    float denominator = m_SpeedOfSound + velocityTowards;
                    if (std::abs(denominator) > 0.1f) {
                        float dopplerPitch = (m_SpeedOfSound - velocityTowards) / denominator;
                        dopplerPitch = glm::clamp(dopplerPitch, 0.5f, 2.0f); // Limit extreme pitch changes
                        source->Pitch = source->Pitch * dopplerPitch * m_DopplerFactor;
                    }
                }

                // Update source properties with 3D calculations
                UpdateSourceProperties(*source);
            }
        }
    }

    AudioHandle AudioManager::GenerateHandle() {
        return m_NextHandle++;
    }

    void AudioManager::CreateMixerChannel(const std::string& name) {
        AudioMixerChannel channel;
        channel.Name = name;
        m_MixerChannels[name] = channel;
    }

    AudioMixerChannel* AudioManager::GetMixerChannel(const std::string& name) {
        auto it = m_MixerChannels.find(name);
        return it != m_MixerChannels.end() ? &it->second : nullptr;
    }

    void AudioManager::CleanupFinishedSources() {
        auto it = m_ActiveSources.begin();
        while (it != m_ActiveSources.end()) {
            if (it->second->State == AudioState::Stopped) {
                // Remove from mixer channels
                for (auto& [channelName, channel] : m_MixerChannels) {
                    auto sourceIt = std::find(channel.Sources.begin(), channel.Sources.end(), it->first);
                    if (sourceIt != channel.Sources.end()) {
                        channel.Sources.erase(sourceIt);
                    }
                }

                DestroySourceInternal(*it->second);

                // Trigger callback
                if (m_OnSoundFinished) {
                    m_OnSoundFinished(it->first);
                }

                it = m_ActiveSources.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    AudioManager::AudioStats AudioManager::GetStatistics() const {
        std::lock_guard<std::mutex> lock(m_AudioMutex);

        AudioStats stats;
        stats.ActiveSources = static_cast<uint32_t>(m_ActiveSources.size());
        stats.LoadedClips = static_cast<uint32_t>(m_AudioClips.size());

        size_t memoryUsage = 0;
        for (const auto& [path, clip] : m_AudioClips) {
            memoryUsage += clip->DataSize;
        }
        stats.MemoryUsage = memoryUsage;

        return stats;
    }

    // Additional missing AudioManager methods

    void AudioManager::StopMusic(float fadeOutTime) {
        if (m_CurrentMusicHandle != INVALID_AUDIO_HANDLE) {
            if (fadeOutTime > 0.0f) {
                auto* source = GetSource(m_CurrentMusicHandle);
                if (source) {
                    source->FadeTarget = 0.0f;
                    source->FadeSpeed = source->Volume / fadeOutTime;
                    source->State = AudioState::Fading;
                }
            } else {
                StopSound(m_CurrentMusicHandle);
            }
            m_CurrentMusicHandle = INVALID_AUDIO_HANDLE;
        }
    }

    void AudioManager::PauseMusic() {
        if (m_CurrentMusicHandle != INVALID_AUDIO_HANDLE) {
            PauseSound(m_CurrentMusicHandle);
        }
    }

    void AudioManager::ResumeMusic() {
        if (m_CurrentMusicHandle != INVALID_AUDIO_HANDLE) {
            ResumeSound(m_CurrentMusicHandle);
        }
    }

    void AudioManager::SetMusicVolume(float volume) {
        m_MusicVolume = glm::clamp(volume, 0.0f, 1.0f);
        if (m_CurrentMusicHandle != INVALID_AUDIO_HANDLE) {
            SetSoundVolume(m_CurrentMusicHandle, m_MusicVolume);
        }
    }

    void AudioManager::UnloadAudioClip(const std::string& path) {
        auto it = m_AudioClips.find(path);
        if (it != m_AudioClips.end()) {
            UnloadClipInternal(*it->second);
            m_AudioClips.erase(it);
            CH_CORE_TRACE("Audio clip unloaded: {}", path);
        }
    }

    bool AudioManager::IsClipLoaded(const std::string& path) const {
        return m_AudioClips.find(path) != m_AudioClips.end();
    }

    const AudioClip* AudioManager::GetClip(const std::string& path) const {
        auto it = m_AudioClips.find(path);
        return it != m_AudioClips.end() ? it->second.get() : nullptr;
    }

    AudioSource* AudioManager::GetSource(AudioHandle handle) {
        auto it = m_ActiveSources.find(handle);
        return it != m_ActiveSources.end() ? it->second.get() : nullptr;
    }

    const AudioSource* AudioManager::GetSource(AudioHandle handle) const {
        auto it = m_ActiveSources.find(handle);
        return it != m_ActiveSources.end() ? it->second.get() : nullptr;
    }

    void AudioManager::StopSound(AudioHandle handle, float fadeOutTime) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (!source) return;

        if (fadeOutTime > 0.0f) {
            source->FadeTarget = 0.0f;
            source->FadeSpeed = source->Volume / fadeOutTime;
            source->State = AudioState::Fading;
        } else {
            StopSourceInternal(*source);
        }
    }

    void AudioManager::PauseSound(AudioHandle handle) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source && source->State == AudioState::Playing) {
            PauseSourceInternal(*source);
            source->State = AudioState::Paused;
        }
    }

    void AudioManager::ResumeSound(AudioHandle handle) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source && source->State == AudioState::Paused) {
            PlaySourceInternal(*source);
            source->State = AudioState::Playing;
        }
    }

    void AudioManager::SetSoundVolume(AudioHandle handle, float volume) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source) {
            source->Volume = glm::clamp(volume, 0.0f, 1.0f);
            UpdateSourceProperties(*source);
        }
    }

    void AudioManager::SetSoundPitch(AudioHandle handle, float pitch) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source) {
            source->Pitch = glm::clamp(pitch, 0.1f, 3.0f);
            UpdateSourceProperties(*source);
        }
    }

    void AudioManager::SetSoundPan(AudioHandle handle, float pan) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source) {
            source->Pan = glm::clamp(pan, -1.0f, 1.0f);
            UpdateSourceProperties(*source);
        }
    }

    void AudioManager::SetSoundLoop(AudioHandle handle, bool loop) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source) {
            source->IsLooping = loop;
            UpdateSourceProperties(*source);
        }
    }

    void AudioManager::SetSound3DPosition(AudioHandle handle, const glm::vec3& position) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source && source->Is3D) {
            source->Settings3D.Position = position;
            UpdateSourceProperties(*source);
        }
    }

    void AudioManager::SetSound3DVelocity(AudioHandle handle, const glm::vec3& velocity) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source && source->Is3D) {
            source->Settings3D.Velocity = velocity;
            UpdateSourceProperties(*source);
        }
    }

    void AudioManager::SetSound3DSettings(AudioHandle handle, const Audio3DSettings& settings) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source && source->Is3D) {
            source->Settings3D = settings;
            UpdateSourceProperties(*source);
        }
    }

    bool AudioManager::IsSoundPlaying(AudioHandle handle) const {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        const auto* source = GetSource(handle);
        return source && source->State == AudioState::Playing;
    }

    AudioState AudioManager::GetSoundState(AudioHandle handle) const {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        const auto* source = GetSource(handle);
        return source ? source->State : AudioState::Stopped;
    }

    float AudioManager::GetSoundVolume(AudioHandle handle) const {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        const auto* source = GetSource(handle);
        return source ? source->Volume : 0.0f;
    }

    float AudioManager::GetSoundPlaybackPosition(AudioHandle handle) const {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        const auto* source = GetSource(handle);
        return source ? source->PlaybackPosition : 0.0f;
    }

    void AudioManager::SetSoundPlaybackPosition(AudioHandle handle, float position) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        auto* source = GetSource(handle);
        if (source) {
            source->PlaybackPosition = std::max(0.0f, position);
            // TODO: Implement platform-specific playback position setting
        }
    }

    void AudioManager::SetListenerPosition(const glm::vec3& position) {
        m_Listener.Position = position;
    }

    void AudioManager::SetListenerVelocity(const glm::vec3& velocity) {
        m_Listener.Velocity = velocity;
    }

    void AudioManager::SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
        m_Listener.Forward = glm::normalize(forward);
        m_Listener.Up = glm::normalize(up);
    }

    void AudioManager::SetMasterVolume(float volume) {
        m_Listener.MasterVolume = glm::clamp(volume, 0.0f, 1.0f);
    }

    void AudioManager::RemoveMixerChannel(const std::string& name) {
        auto it = m_MixerChannels.find(name);
        if (it != m_MixerChannels.end()) {
            // Stop all sources in this channel
            for (AudioHandle handle : it->second.Sources) {
                StopSound(handle);
            }
            m_MixerChannels.erase(it);
            CH_CORE_TRACE("Removed mixer channel: {}", name);
        }
    }

    void AudioManager::SetChannelVolume(const std::string& channel, float volume) {
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->Volume = glm::clamp(volume, 0.0f, 1.0f);
        }
    }

    void AudioManager::SetChannelMute(const std::string& channel, bool muted) {
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->IsMuted = muted;
        }
    }

    void AudioManager::SetChannelLowPassFilter(const std::string& channel, float value) {
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->LowPassFilter = glm::clamp(value, 0.0f, 1.0f);
        }
    }

    void AudioManager::SetChannelHighPassFilter(const std::string& channel, float value) {
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->HighPassFilter = glm::clamp(value, 0.0f, 1.0f);
        }
    }

    void AudioManager::SetChannelReverbLevel(const std::string& channel, float level) {
        auto* mixerChannel = GetMixerChannel(channel);
        if (mixerChannel) {
            mixerChannel->ReverbLevel = glm::clamp(level, 0.0f, 1.0f);
        }
    }

    AudioHandle AudioManager::CreateAudioStream(const std::string& filePath, size_t bufferSize) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);

        AudioHandle handle = GenerateHandle();
        StreamingSource streamingSource;
        streamingSource.FileStream.open(filePath, std::ios::binary);
        
        if (!streamingSource.FileStream.is_open()) {
            CH_CORE_ERROR("Failed to open audio stream: {}", filePath);
            return INVALID_AUDIO_HANDLE;
        }

        streamingSource.BufferSize = bufferSize;
        streamingSource.StreamBuffer.resize(bufferSize);
        
        m_StreamingSources[handle] = std::move(streamingSource);
        
        CH_CORE_TRACE("Created audio stream: {} (handle: {})", filePath, handle);
        return handle;
    }

    void AudioManager::UpdateAudioStream(AudioHandle handle) {
        auto it = m_StreamingSources.find(handle);
        if (it == m_StreamingSources.end()) return;

        auto& streamingSource = it->second;
        if (streamingSource.EndOfFile) return;

        // Read next chunk of data
        streamingSource.FileStream.read(
            reinterpret_cast<char*>(streamingSource.StreamBuffer.data()),
            streamingSource.BufferSize);

        size_t bytesRead = streamingSource.FileStream.gcount();
        if (bytesRead < streamingSource.BufferSize) {
            streamingSource.EndOfFile = true;
        }

        // TODO: Submit buffer to audio system for playback
    }

    void AudioManager::CloseAudioStream(AudioHandle handle) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        
        auto it = m_StreamingSources.find(handle);
        if (it != m_StreamingSources.end()) {
            it->second.FileStream.close();
            m_StreamingSources.erase(it);
            CH_CORE_TRACE("Closed audio stream (handle: {})", handle);
        }
    }

    void AudioManager::SetGlobalReverb(float reverbLevel, float roomSize, float damping) {
        m_GlobalReverbLevel = glm::clamp(reverbLevel, 0.0f, 1.0f);
        // TODO: Apply global reverb settings to audio device
    }

    void AudioManager::SetGlobalLowPassFilter(float cutoffFrequency) {
        m_GlobalLowPassCutoff = glm::clamp(cutoffFrequency, 20.0f, 22050.0f);
        // TODO: Apply global low-pass filter to audio device
    }

    void AudioManager::SetGlobalHighPassFilter(float cutoffFrequency) {
        m_GlobalHighPassCutoff = glm::clamp(cutoffFrequency, 20.0f, 22050.0f);
        // TODO: Apply global high-pass filter to audio device
    }

    std::vector<float> AudioManager::GetSpectrumData(int bands) {
        std::lock_guard<std::mutex> lock(m_AudioMutex);
        
        if (!m_SpectrumAnalysisEnabled || bands <= 0) {
            return std::vector<float>(bands, 0.0f);
        }

        // Resize spectrum data if needed
        if (m_SpectrumData.size() != static_cast<size_t>(bands)) {
            m_SpectrumData.resize(bands, 0.0f);
        }

        return m_SpectrumData;
    }

    float AudioManager::GetRMSLevel(const std::string& channel) {
        // TODO: Calculate RMS level for the specified channel
        // For now, return a placeholder value
        return 0.0f;
    }

    void AudioManager::SetDopplerFactor(float factor) {
        m_DopplerFactor = std::max(0.0f, factor);
    }

    void AudioManager::SetSpeedOfSound(float speed) {
        m_SpeedOfSound = std::max(0.1f, speed);
    }

    void AudioManager::UpdateMixerChannels() {
        // Update mixer channel properties
        for (auto& [name, channel] : m_MixerChannels) {
            // Apply channel effects to all sources in the channel
            for (AudioHandle handle : channel.Sources) {
                auto* source = GetSource(handle);
                if (source) {
                    // Apply channel volume, filters, etc.
                }
            }
        }
    }

    // Platform Implementation Stubs
    bool AudioManager::InitializeAudioDevice() {
#ifdef CH_PLATFORM_WINDOWS
        // Initialize Windows audio device (could use DirectSound, WASAPI, etc.)
        CH_CORE_INFO("Initializing Windows audio device");
        return true;
#elif defined(CH_PLATFORM_LINUX)
        // Initialize ALSA
        CH_CORE_INFO("Initializing ALSA audio device");
        return true;
#else
        CH_CORE_ERROR("Unsupported platform for audio");
        return false;
#endif
    }

    void AudioManager::ShutdownAudioDevice() {
        // Platform-specific cleanup
        CH_CORE_INFO("Audio device shut down");
    }

    bool AudioManager::LoadClipInternal(AudioClip& clip) {
        // Load audio data based on format
        switch (clip.Format) {
            case AudioFormat::WAV:
                return AudioLoader::LoadWAV(clip.Path, clip);
            case AudioFormat::OGG:
                return AudioLoader::LoadOGG(clip.Path, clip);
            case AudioFormat::MP3:
                return AudioLoader::LoadMP3(clip.Path, clip);
            default:
                CH_CORE_ERROR("Unsupported audio format for: {}", clip.Path);
                return false;
        }
    }

    void AudioManager::UnloadClipInternal(AudioClip& clip) {
        clip.Data.clear();
        clip.DataSize = 0;
    }

    AudioHandle AudioManager::CreateSourceInternal(const AudioClip* clip, bool is3D) {
        // Create platform-specific audio source
        AudioHandle handle = GenerateHandle();
        // TODO: Create actual audio source based on platform
        return handle;
    }

    void AudioManager::DestroySourceInternal(AudioSource& source) {
        // Destroy platform-specific audio source
        source.InternalSource = nullptr;
    }

    void AudioManager::PlaySourceInternal(AudioSource& source) {
        // Platform-specific play implementation
    }

    void AudioManager::StopSourceInternal(AudioSource& source) {
        // Platform-specific stop implementation
        source.State = AudioState::Stopped;
    }

    void AudioManager::PauseSourceInternal(AudioSource& source) {
        // Platform-specific pause implementation
    }

    void AudioManager::UpdateSourceProperties(AudioSource& source) {
        // Update platform-specific source properties (volume, pitch, position, etc.)
    }

    // AudioLoader Implementation
    AudioFormat AudioLoader::GetFormatFromExtension(const std::string& filePath) {
        std::string extension = std::filesystem::path(filePath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == ".wav") return AudioFormat::WAV;
        if (extension == ".ogg") return AudioFormat::OGG;
        if (extension == ".mp3") return AudioFormat::MP3;
        if (extension == ".flac") return AudioFormat::FLAC;

        return AudioFormat::Unknown;
    }

    bool AudioLoader::LoadWAV(const std::string& filePath, AudioClip& clip) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            CH_CORE_ERROR("Failed to open WAV file: {}", filePath);
            return false;
        }

        // Read RIFF header
        char riffHeader[12];
        file.read(riffHeader, 12);
        
        if (strncmp(riffHeader, "RIFF", 4) != 0 || strncmp(riffHeader + 8, "WAVE", 4) != 0) {
            CH_CORE_ERROR("Invalid WAV file format: {}", filePath);
            return false;
        }

        // Find fmt chunk
        char chunkId[4];
        uint32_t chunkSize;
        bool foundFmt = false;
        
        while (file.read(chunkId, 4) && file.read(reinterpret_cast<char*>(&chunkSize), 4)) {
            if (strncmp(chunkId, "fmt ", 4) == 0) {
                foundFmt = true;
                break;
            } else {
                // Skip this chunk
                file.seekg(chunkSize, std::ios::cur);
            }
        }

        if (!foundFmt) {
            CH_CORE_ERROR("No fmt chunk found in WAV file: {}", filePath);
            return false;
        }

        // Read format chunk
        uint16_t audioFormat, channels, bitsPerSample;
        uint32_t sampleRate, byteRate;
        uint16_t blockAlign;

        file.read(reinterpret_cast<char*>(&audioFormat), 2);
        file.read(reinterpret_cast<char*>(&channels), 2);
        file.read(reinterpret_cast<char*>(&sampleRate), 4);
        file.read(reinterpret_cast<char*>(&byteRate), 4);
        file.read(reinterpret_cast<char*>(&blockAlign), 2);
        file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

        // Skip any extra format bytes
        if (chunkSize > 16) {
            file.seekg(chunkSize - 16, std::ios::cur);
        }

        // Validate format
        if (audioFormat != 1) { // Only support PCM
            CH_CORE_ERROR("Unsupported WAV format (not PCM): {}", filePath);
            return false;
        }

        // Find data chunk
        bool foundData = false;
        uint32_t dataSize = 0;

        while (file.read(chunkId, 4) && file.read(reinterpret_cast<char*>(&chunkSize), 4)) {
            if (strncmp(chunkId, "data", 4) == 0) {
                foundData = true;
                dataSize = chunkSize;
                break;
            } else {
                // Skip this chunk
                file.seekg(chunkSize, std::ios::cur);
            }
        }

        if (!foundData) {
            CH_CORE_ERROR("No data chunk found in WAV file: {}", filePath);
            return false;
        }

        // Set clip properties
        clip.SampleRate = sampleRate;
        clip.Channels = channels;
        clip.BitsPerSample = bitsPerSample;
        clip.DataSize = dataSize;
        clip.Duration = static_cast<float>(dataSize) / (sampleRate * channels * (bitsPerSample / 8));

        // Read audio data
        clip.Data.resize(dataSize);
        if (!file.read(reinterpret_cast<char*>(clip.Data.data()), dataSize)) {
            CH_CORE_ERROR("Failed to read audio data from WAV file: {}", filePath);
            return false;
        }

        CH_CORE_INFO("Loaded WAV file: {} ({}s, {}Hz, {} channels)", filePath, clip.Duration, clip.SampleRate, clip.Channels);
        return true;
    }

    bool AudioLoader::LoadOGG(const std::string& filePath, AudioClip& clip) {
        // TODO: Implement OGG loading using libvorbis
        CH_CORE_WARN("OGG loading not implemented yet: {}", filePath);
        return false;
    }

    bool AudioLoader::LoadMP3(const std::string& filePath, AudioClip& clip) {
        // TODO: Implement MP3 loading using libmp3lame or similar
        CH_CORE_WARN("MP3 loading not implemented yet: {}", filePath);
        return false;
    }

    // AudioSystem Implementation
    void AudioSystem::OnUpdate(Scene* scene, float deltaTime) {
        auto view = scene->GetAllEntitiesWith<AudioSourceComponent, TransformComponent>();
        
        for (auto entity : view) {
            auto [audio, transform] = view.get<AudioSourceComponent, TransformComponent>(entity);
            
            // Update 3D position if needed
            if (audio.Is3D && audio.Handle != INVALID_AUDIO_HANDLE) {
                AudioManager::Get().SetSound3DPosition(audio.Handle, transform.Translation);
            }
            
            // Auto-play on awake
            if (audio.PlayOnAwake && audio.Handle == INVALID_AUDIO_HANDLE && !audio.ClipPath.empty()) {
                PlayAudio(Entity{entity, scene});
            }
        }
    }

    void AudioSystem::PlayAudio(Entity entity) {
        if (!entity.HasComponent<AudioSourceComponent>()) return;
        
        auto& audio = entity.GetComponent<AudioSourceComponent>();
        
        if (audio.Handle != INVALID_AUDIO_HANDLE) {
            AudioManager::Get().StopSound(audio.Handle);
        }
        
        if (audio.Is3D && entity.HasComponent<TransformComponent>()) {
            auto& transform = entity.GetComponent<TransformComponent>();
            audio.Handle = AudioManager::Get().PlaySound3DOnChannel(
                audio.ClipPath, audio.MixerChannel, transform.Translation, 
                audio.Volume, audio.Pitch, audio.IsLooping);
        } else {
            audio.Handle = AudioManager::Get().PlaySoundOnChannel(
                audio.ClipPath, audio.MixerChannel, 
                audio.Volume, audio.Pitch, audio.IsLooping);
        }
    }

    void AudioSystem::StopAudio(Entity entity) {
        if (!entity.HasComponent<AudioSourceComponent>()) return;
        
        auto& audio = entity.GetComponent<AudioSourceComponent>();
        
        if (audio.Handle != INVALID_AUDIO_HANDLE) {
            AudioManager::Get().StopSound(audio.Handle);
            audio.Handle = INVALID_AUDIO_HANDLE;
        }
    }

    void AudioSystem::PauseAudio(Entity entity) {
        if (!entity.HasComponent<AudioSourceComponent>()) return;
        
        auto& audio = entity.GetComponent<AudioSourceComponent>();
        
        if (audio.Handle != INVALID_AUDIO_HANDLE) {
            AudioManager::Get().PauseSound(audio.Handle);
        }
    }

    void AudioSystem::OnEntityCreated(Entity entity, AudioSourceComponent& component) {
        // Initialize any necessary data when an audio component is added
        component.Handle = INVALID_AUDIO_HANDLE;
    }

    void AudioSystem::OnEntityDestroyed(Entity entity, AudioSourceComponent& component) {
        // Clean up audio source when entity is destroyed
        StopAudio(entity);
    }

    void AudioSystem::UpdateAudioSources(Scene* scene) {
        // Additional audio source management if needed
    }

    void AudioSystem::Update3DPosition(Entity entity, AudioSourceComponent& audioComp, const glm::vec3& position) {
        if (audioComp.Is3D && audioComp.Handle != INVALID_AUDIO_HANDLE) {
            AudioManager::Get().SetSound3DPosition(audioComp.Handle, position);
        }
    }

    // AudioEffects Implementation
    void AudioEffects::ApplyVolume(std::vector<int16_t>& samples, float volume) {
        for (int16_t& sample : samples) {
            sample = static_cast<int16_t>(sample * volume);
        }
    }

    void AudioEffects::ApplyFade(std::vector<int16_t>& samples, float startVolume, float endVolume) {
        size_t sampleCount = samples.size();
        if (sampleCount == 0) return;
        
        for (size_t i = 0; i < sampleCount; ++i) {
            float t = (sampleCount > 1) ? static_cast<float>(i) / static_cast<float>(sampleCount - 1) : 0.0f;
            float volume = startVolume + (endVolume - startVolume) * t;
            samples[i] = static_cast<int16_t>(samples[i] * volume);
        }
    }

    void AudioEffects::ApplyLowPassFilter(std::vector<int16_t>& samples, float cutoff, uint32_t sampleRate) {
        // Simple first-order low-pass filter
        float rc = 1.0f / (cutoff * 2.0f * 3.14159f);
        float dt = 1.0f / sampleRate;
        float alpha = dt / (rc + dt);
        
        float filteredValue = samples[0];
        for (size_t i = 1; i < samples.size(); ++i) {
            filteredValue = filteredValue + alpha * (samples[i] - filteredValue);
            samples[i] = static_cast<int16_t>(filteredValue);
        }
    }

    void AudioEffects::ApplyHighPassFilter(std::vector<int16_t>& samples, float cutoff, uint32_t sampleRate) {
        // Simple first-order high-pass filter
        float rc = 1.0f / (cutoff * 2.0f * 3.14159f);
        float dt = 1.0f / sampleRate;
        float alpha = rc / (rc + dt);
        
        int16_t prevInput = samples[0];
        int16_t prevOutput = samples[0];
        
        for (size_t i = 1; i < samples.size(); ++i) {
            int16_t currentInput = samples[i];
            int16_t currentOutput = static_cast<int16_t>(alpha * (prevOutput + currentInput - prevInput));
            
            samples[i] = currentOutput;
            prevInput = currentInput;
            prevOutput = currentOutput;
        }
    }

    void AudioEffects::ApplyReverb(std::vector<int16_t>& samples, float roomSize, float damping, float wetLevel) {
        // Simple delay-based reverb effect
        size_t delayLength = static_cast<size_t>(roomSize * 0.1f * samples.size());
        if (delayLength >= samples.size()) delayLength = samples.size() - 1;
        
        std::vector<int16_t> delayBuffer(delayLength, 0);
        size_t delayIndex = 0;
        
        for (size_t i = 0; i < samples.size(); ++i) {
            int16_t delayed = delayBuffer[delayIndex];
            int16_t dampedDelayed = static_cast<int16_t>(delayed * damping);
            
            delayBuffer[delayIndex] = static_cast<int16_t>(samples[i] + dampedDelayed * 0.3f);
            
            samples[i] = static_cast<int16_t>(samples[i] * (1.0f - wetLevel) + dampedDelayed * wetLevel);
            
            delayIndex = (delayIndex + 1) % delayLength;
        }
    }

    void AudioEffects::ApplyEcho(std::vector<int16_t>& samples, float delay, float decay, uint32_t sampleRate) {
        size_t delayInSamples = static_cast<size_t>(delay * sampleRate);
        if (delayInSamples >= samples.size()) return;
        
        for (size_t i = delayInSamples; i < samples.size(); ++i) {
            int16_t echoSample = static_cast<int16_t>(samples[i - delayInSamples] * decay);
            samples[i] = static_cast<int16_t>(samples[i] + echoSample);
        }
    }

    std::vector<float> AudioEffects::ComputeSpectrum(const std::vector<int16_t>& samples, int bands) {
        std::vector<float> spectrum(bands, 0.0f);
        
        if (samples.empty() || bands <= 0) return spectrum;
        
        // Simple magnitude spectrum computation
        size_t samplesPerBand = samples.size() / bands;
        if (samplesPerBand == 0) samplesPerBand = 1;
        
        for (int band = 0; band < bands; ++band) {
            float magnitude = 0.0f;
            size_t startIdx = band * samplesPerBand;
            size_t endIdx = std::min(startIdx + samplesPerBand, samples.size());
            
            if (endIdx > startIdx) {
                for (size_t i = startIdx; i < endIdx; ++i) {
                    magnitude += std::abs(samples[i]);
                }
                spectrum[band] = magnitude / (endIdx - startIdx);
            }
        }
        
        return spectrum;
    }

    float AudioEffects::ComputeRMS(const std::vector<int16_t>& samples) {
        if (samples.empty()) return 0.0f;
        
        float sumSquares = 0.0f;
        for (int16_t sample : samples) {
            float normalized = sample / 32767.0f;
            sumSquares += normalized * normalized;
        }
        
        return std::sqrt(sumSquares / samples.size());
    }

} // namespace Cherry