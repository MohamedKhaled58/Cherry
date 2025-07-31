#include "CHpch.h"
#include "ParticleSystem.h"
#include "Cherry/Renderer/Renderer2D.h"
#include "Cherry/Renderer/Camera.h"
#include <random>

namespace Cherry {

    void ParticleSystem::Initialize(uint32_t maxParticles) {
        m_MaxParticles = maxParticles;
        m_Particles.resize(maxParticles);
        m_ActiveParticles = 0;
        m_IsActive = false;
        m_EmissionTimer = 0.0f;
    }

    void ParticleSystem::Update(float deltaTime) {
        if (!m_IsActive) return;

        // Update emission timer
        m_EmissionTimer += deltaTime;
        float emissionInterval = 1.0f / m_EmissionRate;

        // Emit new particles
        while (m_EmissionTimer >= emissionInterval && m_ActiveParticles < m_MaxParticles) {
            EmitParticle();
            m_EmissionTimer -= emissionInterval;
        }

        // Update existing particles
        for (auto& particle : m_Particles) {
            if (particle.Active) {
                UpdateParticle(particle, deltaTime);
            }
        }
    }

    void ParticleSystem::Render(const OrthographicCamera& camera) {
        if (m_ActiveParticles == 0) return;

        // Simple particle rendering using Renderer2D
        for (const auto& particle : m_Particles) {
            if (particle.Active) {
                float alpha = particle.Life / particle.MaxLife;
                glm::vec4 color = particle.Color;
                color.a *= alpha;

                Renderer2D::DrawQuad(
                    particle.Position,
                    0.0f, // No rotation
                    { particle.Size, particle.Size },
                    color
                );
            }
        }
    }

    void ParticleSystem::EmitParticle() {
        // Find an inactive particle
        for (auto& particle : m_Particles) {
            if (!particle.Active) {
                // Initialize particle
                particle.Position = m_Position;
                particle.Velocity = m_Velocity;
                
                // Add random variation to velocity
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> velVar(-m_VelocityVariation, m_VelocityVariation);
                
                particle.Velocity.x += velVar(gen);
                particle.Velocity.y += velVar(gen);
                particle.Velocity.z += velVar(gen);

                particle.Color = m_Color;
                particle.Size = m_ParticleSize;
                particle.Life = m_ParticleLifetime;
                particle.MaxLife = m_ParticleLifetime;
                particle.Active = true;

                m_ActiveParticles++;
                break;
            }
        }
    }

    void ParticleSystem::UpdateParticle(Particle& particle, float deltaTime) {
        // Update position
        particle.Position += particle.Velocity * deltaTime;

        // Update life
        particle.Life -= deltaTime;

        // Deactivate if dead
        if (particle.Life <= 0.0f) {
            particle.Active = false;
            m_ActiveParticles--;
        }
    }

    void ParticleSystem::Reset() {
        for (auto& particle : m_Particles) {
            particle.Active = false;
        }
        m_ActiveParticles = 0;
        m_EmissionTimer = 0.0f;
    }

} // namespace Cherry 