#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Cherry {
    class EffectSystem {
    public:
        // Particle effects for skills/spells
        EffectHandle CreateEffect(const std::string& effectName);
        void PlayEffect(EffectHandle handle, const glm::vec3& position);
        void StopEffect(EffectHandle handle);

        // Post-processing effects
        void AddPostEffect(std::unique_ptr<PostProcessEffect> effect);
        void RemovePostEffect(const std::string& name);

    private:
        ParticleSystem m_ParticleSystem;
        std::vector<std::unique_ptr<PostProcessEffect>> m_PostEffects;
    };
}