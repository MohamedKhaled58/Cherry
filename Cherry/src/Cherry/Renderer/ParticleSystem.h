#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace Cherry {

    struct Particle {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };
        glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        float Size = 1.0f;
        float Life = 1.0f;
        float MaxLife = 1.0f;
        bool Active = false;
    };

    class ParticleSystem {
    public:
        ParticleSystem() = default;
        ~ParticleSystem() = default;

        void Initialize(uint32_t maxParticles = 1000);
        void Update(float deltaTime);
        void Render(const class OrthographicCamera& camera);

        // Emission control
        void SetEmissionRate(float rate) { m_EmissionRate = rate; }
        void SetLifetime(float lifetime) { m_ParticleLifetime = lifetime; }
        void SetVelocity(const glm::vec3& velocity) { m_Velocity = velocity; }
        void SetVelocityVariation(float variation) { m_VelocityVariation = variation; }
        void SetColor(const glm::vec4& color) { m_Color = color; }
        void SetSize(float size) { m_ParticleSize = size; }

        // System control
        void Start() { m_IsActive = true; }
        void Stop() { m_IsActive = false; }
        void Reset();

        bool IsActive() const { return m_IsActive; }
        uint32_t GetActiveParticleCount() const { return m_ActiveParticles; }

    private:
        void EmitParticle();
        void UpdateParticle(Particle& particle, float deltaTime);

    private:
        std::vector<Particle> m_Particles;
        uint32_t m_MaxParticles = 1000;
        uint32_t m_ActiveParticles = 0;

        // Emission properties
        float m_EmissionRate = 10.0f; // particles per second
        float m_EmissionTimer = 0.0f;
        float m_ParticleLifetime = 2.0f;
        glm::vec3 m_Velocity = { 0.0f, 0.0f, 0.0f };
        float m_VelocityVariation = 0.5f;
        glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        float m_ParticleSize = 1.0f;

        // System state
        bool m_IsActive = false;
        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
    };

} // namespace Cherry 