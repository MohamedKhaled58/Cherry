#pragma once
#include "Cherry/Core/Core.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <Cherry/Scene/Scene.h>
#include <Cherry/Scene/Entity.h>

namespace Cherry {

    using AudioHandle = uint32_t;
    constexpr AudioHandle INVALID_AUDIO_HANDLE = 0;

    enum class AudioState {
        Stopped,
        Playing,
        Paused,
        Fading
    };

    enum class AudioFormat {
        Unknown,
        WAV,
        OGG,
        MP3,
        FLAC
    };

    struct AudioClip {
        std::string Path;
        AudioFormat Format;
        uint32_t SampleRate;
        uint16_t Channels;
        uint16_t BitsPerSample;
        float Duration;
        size_t DataSize;
        std::vector<uint8_t> Data;
        bool IsStreaming = false;

        AudioClip() = default;
        AudioClip(const std::string& path) : Path(path) {}
    };

    struct Audio3DSettings {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };
        float MinDistance = 1.0f;
        float MaxDistance = 100.0f;
        float RolloffFactor = 1.0f;
        bool EnableDoppler = true;
    };

    struct AudioSource {
        AudioHandle Handle;
        std::string ClipPath;
        AudioState State = AudioState::Stopped;

        // Playback properties
        float Volume = 1.0f;
        float Pitch = 1.0f;
        float Pan = 0.0f; // -1.0 (left) to 1.0 (right)
        bool IsLooping = false;

        // 3D Audio
        bool Is3D = false;
        Audio3DSettings Settings3D;

        // Timing
        float PlaybackPosition = 0.0f;
        float FadeTarget = 1.0f;
        float FadeSpeed = 1.0f;

        // Internal
        void* InternalSource = nullptr; // Platform-specific audio source

        AudioSource(AudioHandle handle, const std::string& clipPath)
            : Handle(handle), ClipPath(clipPath) {
        }
    };

    struct AudioListener {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Forward = { 0.0f, 0.0f, -1.0f };
        glm::vec3 Up = { 0.0f, 1.0f, 0.0f };
        float MasterVolume = 1.0f;
    };

    struct AudioMixerChannel {
        std::string Name;
        float Volume = 1.0f;
        float LowPassFilter = 1.0f;  // 0.0 to 1.0
        float HighPassFilter = 0.0f; // 0.0 to 1.0
        float ReverbLevel = 0.0f;    // 0.0 to 1.0
        bool IsMuted = false;

        std::vector<AudioHandle> Sources;
    };

    class AudioManager {
    public:
        static AudioManager& Get() {
            static AudioManager instance;
            return instance;
        }

        // Initialization
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return m_IsInitialized; }

        // Audio clip management
        bool LoadAudioClip(const std::string& path, bool preload = true);
        void UnloadAudioClip(const std::string& path);
        bool IsClipLoaded(const std::string& path) const;
        const AudioClip* GetClip(const std::string& path) const;

        // 2D Audio playback
        AudioHandle PlaySound(const std::string& clipPath, float volume = 1.0f, float pitch = 1.0f, bool loop = false);
        AudioHandle PlaySoundOnChannel(const std::string& clipPath, const std::string& channel,
            float volume = 1.0f, float pitch = 1.0f, bool loop = false);

        // 3D Audio playback
        AudioHandle PlaySound3D(const std::string& clipPath, const glm::vec3& position,
            float volume = 1.0f, float pitch = 1.0f, bool loop = false);
        AudioHandle PlaySound3DOnChannel(const std::string& clipPath, const std::string& channel,
            const glm::vec3& position, float volume = 1.0f,
            float pitch = 1.0f, bool loop = false);

        // Music system
        void PlayMusic(const std::string& musicPath, float fadeInTime = 0.0f, bool loop = true);
        void StopMusic(float fadeOutTime = 0.0f);
        void PauseMusic();
        void ResumeMusic();
        void SetMusicVolume(float volume);
        float GetMusicVolume() const { return m_MusicVolume; }

        // Source control
        void StopSound(AudioHandle handle, float fadeOutTime = 0.0f);
        void PauseSound(AudioHandle handle);
        void ResumeSound(AudioHandle handle);
        void SetSoundVolume(AudioHandle handle, float volume);
        void SetSoundPitch(AudioHandle handle, float pitch);
        void SetSoundPan(AudioHandle handle, float pan);
        void SetSoundLoop(AudioHandle handle, bool loop);

        // 3D Audio source control
        void SetSound3DPosition(AudioHandle handle, const glm::vec3& position);
        void SetSound3DVelocity(AudioHandle handle, const glm::vec3& velocity);
        void SetSound3DSettings(AudioHandle handle, const Audio3DSettings& settings);

        // Source queries
        bool IsSoundPlaying(AudioHandle handle) const;
        AudioState GetSoundState(AudioHandle handle) const;
        float GetSoundVolume(AudioHandle handle) const;
        float GetSoundPlaybackPosition(AudioHandle handle) const;
        void SetSoundPlaybackPosition(AudioHandle handle, float position);

        // Listener (camera/player position)
        void SetListenerPosition(const glm::vec3& position);
        void SetListenerVelocity(const glm::vec3& velocity);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
        const AudioListener& GetListener() const { return m_Listener; }

        // Master volume control
        void SetMasterVolume(float volume);
        float GetMasterVolume() const { return m_Listener.MasterVolume; }

        // Mixer channels
        void CreateMixerChannel(const std::string& name);
        void RemoveMixerChannel(const std::string& name);
        void SetChannelVolume(const std::string& channel, float volume);
        void SetChannelMute(const std::string& channel, bool muted);
        void SetChannelLowPassFilter(const std::string& channel, float value);
        void SetChannelHighPassFilter(const std::string& channel, float value);
        void SetChannelReverbLevel(const std::string& channel, float level);
        AudioMixerChannel* GetMixerChannel(const std::string& name);

        // Audio streaming
        AudioHandle CreateAudioStream(const std::string& filePath, size_t bufferSize = 4096);
        void UpdateAudioStream(AudioHandle handle);
        void CloseAudioStream(AudioHandle handle);

        // Effects and filters
        void SetGlobalReverb(float reverbLevel, float roomSize = 0.5f, float damping = 0.5f);
        void SetGlobalLowPassFilter(float cutoffFrequency);
        void SetGlobalHighPassFilter(float cutoffFrequency);

        // Audio analysis
        void EnableSpectrumAnalysis(bool enable) { m_SpectrumAnalysisEnabled = enable; }
        std::vector<float> GetSpectrumData(int bands = 64);
        float GetRMSLevel(const std::string& channel = "");

        // Update (call every frame)
        void Update(float deltaTime);

        // Events
        void SetOnMusicFinished(std::function<void()> callback) { m_OnMusicFinished = callback; }
        void SetOnSoundFinished(std::function<void(AudioHandle)> callback) { m_OnSoundFinished = callback; }

        // Configuration
        void SetMaxActiveSources(uint32_t maxSources) { m_MaxActiveSources = maxSources; }
        void SetDopplerFactor(float factor);
        void SetSpeedOfSound(float speed);

        // Debug and statistics
        struct AudioStats {
            uint32_t ActiveSources = 0;
            uint32_t LoadedClips = 0;
            size_t MemoryUsage = 0;
            float CPUUsage = 0.0f;
        };
        AudioStats GetStatistics() const;

    private:
        AudioManager() = default;
        ~AudioManager() { Shutdown(); }

        // Internal methods
        AudioHandle GenerateHandle();
        AudioSource* GetSource(AudioHandle handle);
        const AudioSource* GetSource(AudioHandle handle) const;
        void CleanupFinishedSources();
        void UpdateFading(float deltaTime);
        void Update3DAudio();
        void UpdateMixerChannels();

        // Platform-specific implementations
        bool InitializeAudioDevice();
        void ShutdownAudioDevice();
        bool LoadClipInternal(AudioClip& clip);
        void UnloadClipInternal(AudioClip& clip);
        AudioHandle CreateSourceInternal(const AudioClip* clip, bool is3D);
        void DestroySourceInternal(AudioSource& source);
        void PlaySourceInternal(AudioSource& source);
        void StopSourceInternal(AudioSource& source);
        void PauseSourceInternal(AudioSource& source);
        void UpdateSourceProperties(AudioSource& source);

    private:
        bool m_IsInitialized = false;
        void* m_AudioDevice = nullptr; // Platform-specific audio device
        void* m_AudioContext = nullptr; // Platform-specific audio context

        // Audio clips cache
        std::unordered_map<std::string, std::unique_ptr<AudioClip>> m_AudioClips;

        // Active audio sources
        std::unordered_map<AudioHandle, std::unique_ptr<AudioSource>> m_ActiveSources;
        uint32_t m_NextHandle = 1;
        uint32_t m_MaxActiveSources = 32;

        // Audio listener
        AudioListener m_Listener;

        // Music system
        AudioHandle m_CurrentMusicHandle = INVALID_AUDIO_HANDLE;
        float m_MusicVolume = 1.0f;
        std::string m_CurrentMusicPath;

        // Mixer channels
        std::unordered_map<std::string, AudioMixerChannel> m_MixerChannels;

        // Streaming
        struct StreamingSource {
            std::ifstream FileStream;
            size_t BufferSize;
            std::vector<uint8_t> StreamBuffer;
            bool EndOfFile = false;
        };
        std::unordered_map<AudioHandle, StreamingSource> m_StreamingSources;

        // Audio analysis
        bool m_SpectrumAnalysisEnabled = false;
        std::vector<float> m_SpectrumData;

        // Global effects
        float m_GlobalReverbLevel = 0.0f;
        float m_GlobalLowPassCutoff = 22050.0f;
        float m_GlobalHighPassCutoff = 20.0f;
        float m_DopplerFactor = 1.0f;
        float m_SpeedOfSound = 343.3f; // m/s

        // Event callbacks
        std::function<void()> m_OnMusicFinished;
        std::function<void(AudioHandle)> m_OnSoundFinished;

        // Threading
        mutable std::mutex m_AudioMutex;
        std::thread m_AudioThread;
        std::atomic<bool> m_AudioThreadRunning = false;
    };

    // Audio utilities and helper classes
    class AudioLoader {
    public:
        static bool LoadWAV(const std::string& filePath, AudioClip& clip);
        static bool LoadOGG(const std::string& filePath, AudioClip& clip);
        static bool LoadMP3(const std::string& filePath, AudioClip& clip);
        static AudioFormat GetFormatFromExtension(const std::string& filePath);

    private:
        static bool ReadWAVHeader(std::ifstream& file, AudioClip& clip);
        static bool ReadOGGHeader(std::ifstream& file, AudioClip& clip);
    };

    class AudioEffects {
    public:
        // Simple audio effects that can be applied to audio data
        static void ApplyVolume(std::vector<int16_t>& samples, float volume);
        static void ApplyFade(std::vector<int16_t>& samples, float startVolume, float endVolume);
        static void ApplyLowPassFilter(std::vector<int16_t>& samples, float cutoff, uint32_t sampleRate);
        static void ApplyHighPassFilter(std::vector<int16_t>& samples, float cutoff, uint32_t sampleRate);
        static void ApplyReverb(std::vector<int16_t>& samples, float roomSize, float damping, float wetLevel);
        static void ApplyEcho(std::vector<int16_t>& samples, float delay, float decay, uint32_t sampleRate);

        // Spectrum analysis
        static std::vector<float> ComputeSpectrum(const std::vector<int16_t>& samples, int bands);
        static float ComputeRMS(const std::vector<int16_t>& samples);
    };

    // Forward declaration - AudioSourceComponent is defined in Components.h
    struct AudioSourceComponent;

    // Audio system for ECS
    class AudioSystem {
    public:
        void OnUpdate(Scene* scene, float deltaTime);
        void OnEntityCreated(Entity entity, AudioSourceComponent& component);
        void OnEntityDestroyed(Entity entity, AudioSourceComponent& component);

        // Control methods
        void PlayAudio(Entity entity);
        void StopAudio(Entity entity);
        void PauseAudio(Entity entity);

    private:
        void UpdateAudioSources(Scene* scene);
        void Update3DPosition(Entity entity, AudioSourceComponent& audioComp, const glm::vec3& position);
    };

} // namespace Cherry
