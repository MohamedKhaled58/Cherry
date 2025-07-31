// Cherry/src/Cherry/Scene/Scene.cpp
#include "CHpch.h"
#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Cherry/Renderer/Renderer2D.h"
#include "Cherry/Audio/AudioManager.h"

namespace Cherry {

    Scene::Scene() {
        CH_CORE_INFO("Scene created");
    }

    Scene::~Scene() {
        CH_CORE_INFO("Scene destroyed");
    }

    Entity Scene::CreateEntity(const std::string& name) {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    Entity Scene::CreateEntityWithUUID(const std::string& name) {
        Entity entity = CreateEntity(name);
        entity.AddComponent<IDComponent>();
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }

    void Scene::OnUpdateRuntime(TimeStep ts) {
        // Update scripts
        {
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
                if (!nsc.Instance) {
                    nsc.Instance = nsc.InstantiateScript();
                    nsc.Instance->m_Entity = Entity{ entity, this };
                    nsc.Instance->OnCreate();
                }
                nsc.Instance->OnUpdate(ts);
                });
        }

        // Update animations
        {
            auto view = m_Registry.view<AnimationComponent>();
            for (auto entity : view) {
                auto& animComp = view.get<AnimationComponent>(entity);
                if (animComp.IsPlaying) {
                    animComp.AnimationTime += ts * animComp.AnimationSpeed;

                    // Check if animation has finished
                    auto animIt = animComp.Animations.find(animComp.CurrentAnimation);
                    if (animIt != animComp.Animations.end()) {
                        if (animComp.AnimationTime >= animIt->second.Duration) {
                            if (animComp.IsLooping) {
                                animComp.AnimationTime = 0.0f;
                            }
                            else {
                                animComp.IsPlaying = false;
                                animComp.AnimationTime = animIt->second.Duration;
                            }
                        }
                    }
                }
            }
        }

        // Update movement
        {
            auto view = m_Registry.view<MovementComponent, TransformComponent>();
            for (auto entity : view) {
                auto [movement, transform] = view.get<MovementComponent, TransformComponent>(entity);

                if (movement.IsMoving && movement.HasDestination) {
                    glm::vec3 direction = glm::vec3(movement.Destination.x - transform.Translation.x,
                        movement.Destination.y - transform.Translation.y, 0.0f);
                    float distance = glm::length(direction);

                    if (distance < 0.1f) {
                        // Reached destination
                        movement.Stop();
                        transform.Translation.x = movement.Destination.x;
                        transform.Translation.y = movement.Destination.y;
                    }
                    else {
                        // Move towards destination
                        direction = glm::normalize(direction);
                        movement.Velocity = glm::vec2(direction.x, direction.y) * movement.Speed;
                        transform.Translation.x += movement.Velocity.x * ts;
                        transform.Translation.y += movement.Velocity.y * ts;
                    }
                }
            }
        }

        // Update audio sources
        {
            auto view = m_Registry.view<AudioSourceComponent, TransformComponent>();
            for (auto entity : view) {
                auto [audio, transform] = view.get<AudioSourceComponent, TransformComponent>(entity);

                if (audio.Is3D) {
                    // Update 3D audio position
                    // This would integrate with the AudioManager
                }
            }
        }

        // Update network synchronization
        {
            auto view = m_Registry.view<NetworkComponent, TransformComponent>();
            for (auto entity : view) {
                auto [network, transform] = view.get<NetworkComponent, TransformComponent>(entity);

                // Check if position has changed significantly
                float distance = glm::distance(network.LastSyncedPosition, transform.Translation);
                if (distance > 0.1f) { // Threshold for network updates
                    network.IsDirty = true;
                    network.LastSyncedPosition = transform.Translation;
                    network.LastSyncTime = ts;
                }
            }
        }

        // Render the scene
        OrthographicCamera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : view) {
                auto& transform = view.get<TransformComponent>(entity);
                auto& camera = view.get<CameraComponent>(entity);
                if (camera.Primary) {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    break;
                }
            }
        }

        if (mainCamera) {
            RenderScene(*mainCamera);
        }
    }

    void Scene::OnUpdateEditor(TimeStep ts, OrthographicCamera& camera) {
        RenderScene(camera);
    }

    void Scene::RenderScene(OrthographicCamera& camera) {
        Renderer2D::BeginScene(camera);

        // Render sprite components
        {
            auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
            for (auto entity : view) {
                auto& transform = view.get<TransformComponent>(entity);
                auto& sprite = view.get<SpriteRendererComponent>(entity);

                if (sprite.Texture) {
                    Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture,
                        sprite.TilingFactor, sprite.Color);
                }
                else {
                    Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
                }
            }
        }

        // Render circle components
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view) {
                auto& transform = view.get<TransformComponent>(entity);
                auto& circle = view.get<CircleRendererComponent>(entity);

                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade);
            }
        }

        Renderer2D::EndScene();
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // Resize non-fixed aspect ratio cameras
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view) {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio) {
                cameraComponent.Camera.SetProjection(-static_cast<float>(width) / 2.0f,
                    static_cast<float>(width) / 2.0f,
                    -static_cast<float>(height) / 2.0f,
                    static_cast<float>(height) / 2.0f);
            }
        }
    }

    Entity Scene::GetPrimaryCameraEntity() {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view) {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.Primary)
                return Entity{ entity, this };
        }
        return {};
    }

    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component) {
        // Template specialization will be defined below
    }

    // Template specializations
    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {
        // No special handling needed
    }

    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {
        if (m_ViewportWidth > 0 && m_ViewportHeight > 0) {
            component.Camera.SetProjection(-static_cast<float>(m_ViewportWidth) / 2.0f,
                static_cast<float>(m_ViewportWidth) / 2.0f,
                -static_cast<float>(m_ViewportHeight) / 2.0f,
                static_cast<float>(m_ViewportHeight) / 2.0f);
        }
    }

    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) {
        // No special handling needed
    }

    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) {
        // No special handling needed
    }

    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) {
        // Script will be instantiated during OnUpdateRuntime
    }

    template<>
    void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component) {
        if (component.PlayOnAwake && !component.ClipPath.empty()) {
            // Start playing audio
            component.Handle = AudioManager::Get().PlaySound(component.ClipPath,
                component.Volume,
                component.Pitch,
                component.IsLooping);
        }
    }

    template<>
    void Scene::OnComponentAdded<NetworkComponent>(Entity entity, NetworkComponent& component) {
        // Initialize network sync data
        auto& transform = entity.GetComponent<TransformComponent>();
        component.LastSyncedPosition = transform.Translation;
        component.LastSyncTime = 0.0f;
    }

} // namespace Cherry


