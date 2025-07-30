#include "CHpch.h"
#include "AudioManager.h"

// Platform-specific includes would go here
#ifdef CH_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsAudio.h"
#elif defined(CH_PLATFORM_LINUX)
#include "Platform/Linux/LinuxAudio.h"
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

} // namespace Cherry