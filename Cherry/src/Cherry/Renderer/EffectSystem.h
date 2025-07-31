#pragma once
#include "ParticleSystem.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace Cherry {
    // Forward declarations
    using EffectHandle = uint32_t;
    static constexpr EffectHandle INVALID_EFFECT_HANDLE = 0;

    // Base class for post-processing effects
    class PostProcessEffect {
    public:
        virtual ~PostProcessEffect() = default;
        virtual void Apply() = 0;
        virtual const std::string& GetName() const = 0;
        virtual void SetEnabled(bool enabled) { m_Enabled = enabled; }
        virtual bool IsEnabled() const { return m_Enabled; }

    protected:
        bool m_Enabled = true;
        std::string m_Name;
    };

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