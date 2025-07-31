#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "AudioManager.h"
namespace Cherry {
    class AudioManager {
    public:
        // 3D positional audio for MMO environments
        AudioHandle PlaySound(const std::string& soundPath,
            const glm::vec3& position = glm::vec3(0.0f));

        // Background music with crossfading
        void PlayMusic(const std::string& musicPath, float fadeTime = 1.0f);

        // Audio streaming for large files
        AudioHandle StreamAudio(const std::string& audioPath);

        // Listener management
        void SetListenerPosition(const glm::vec3& position);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);

    private:
        std::unique_ptr<AudioDevice> m_AudioDevice;
        std::unordered_map<AudioHandle, AudioSource> m_ActiveSources;
    };
}