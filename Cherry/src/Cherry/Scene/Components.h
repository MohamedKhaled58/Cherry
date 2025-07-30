
#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Renderer/Texture.h"
#include "Cherry/Renderer/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace Cherry {

    struct IDComponent {
        std::string ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
        IDComponent(const std::string& id) : ID(id) {}
    };

    struct TagComponent {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct TransformComponent {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation)
            : Translation(translation) {
        }

        glm::mat4 GetTransform() const {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
                * glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
                * glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    struct SpriteRendererComponent {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        REF(Texture2D) Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {
        }
    };

    struct CircleRendererComponent {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        float Thickness = 1.0f;
        float Fade = 0.005f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
    };

    struct CameraComponent {
        OrthographicCamera Camera;
        bool Primary = true; // TODO: think about moving to Scene
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    // Script component for game logic
    struct NativeScriptComponent {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity* (*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);

        template<typename T>
        void Bind() {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
        }
    };

    // Physics components
    struct RigidBody2DComponent {
        enum class BodyType { Static = 0, Kinematic, Dynamic };
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        // Storage for runtime body
        void* RuntimeBody = nullptr;

        RigidBody2DComponent() = default;
        RigidBody2DComponent(const RigidBody2DComponent&) = default;
    };

    struct BoxCollider2DComponent {
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 0.5f, 0.5f };

        // Physics material properties
        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        // Storage for runtime fixture
        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

    struct CircleCollider2DComponent {
        glm::vec2 Offset = { 0.0f, 0.0f };
        float Radius = 0.5f;

        // Physics material properties
        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        // Storage for runtime fixture
        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    // Animation components
    struct AnimationComponent {
        std::string CurrentAnimation;
        float AnimationTime = 0.0f;
        float AnimationSpeed = 1.0f;
        bool IsPlaying = false;
        bool IsLooping = true;

        // Animation state machine
        std::unordered_map<std::string, AnimationClip> Animations;
        std::unordered_map<std::string, std::string> Transitions;

        AnimationComponent() = default;
        AnimationComponent(const AnimationComponent&) = default;

        void Play(const std::string& animationName, bool loop = true) {
            if (Animations.find(animationName) != Animations.end()) {
                CurrentAnimation = animationName;
                AnimationTime = 0.0f;
                IsPlaying = true;
                IsLooping = loop;
            }
        }

        void Stop() {
            IsPlaying = false;
            AnimationTime = 0.0f;
        }

        void Pause() {
            IsPlaying = false;
        }

        void Resume() {
            IsPlaying = true;
        }
    };

    // Game-specific components for MMO
    struct PlayerComponent {
        std::string PlayerName;
        uint32_t PlayerID = 0;
        uint32_t Level = 1;
        uint32_t Experience = 0;
        uint32_t Health = 100;
        uint32_t MaxHealth = 100;
        uint32_t Mana = 100;
        uint32_t MaxMana = 100;

        PlayerComponent() = default;
        PlayerComponent(const PlayerComponent&) = default;
        PlayerComponent(const std::string& name, uint32_t id)
            : PlayerName(name), PlayerID(id) {
        }
    };

    struct NPCComponent {
        std::string NPCName;
        uint32_t NPCID = 0;
        std::string NPCType; // "monster", "trader", "quest_giver", etc.
        uint32_t Health = 100;
        uint32_t MaxHealth = 100;
        bool IsHostile = false;
        float InteractionRange = 2.0f;

        NPCComponent() = default;
        NPCComponent(const NPCComponent&) = default;
        NPCComponent(const std::string& name, uint32_t id, const std::string& type)
            : NPCName(name), NPCID(id), NPCType(type) {
        }
    };

    struct MovementComponent {
        glm::vec2 Velocity = { 0.0f, 0.0f };
        float Speed = 5.0f;
        float MaxSpeed = 10.0f;
        glm::vec2 Destination = { 0.0f, 0.0f };
        bool HasDestination = false;
        bool IsMoving = false;

        MovementComponent() = default;
        MovementComponent(const MovementComponent&) = default;
        MovementComponent(float speed) : Speed(speed), MaxSpeed(speed * 2.0f) {}

        void SetDestination(const glm::vec2& dest) {
            Destination = dest;
            HasDestination = true;
            IsMoving = true;
        }

        void Stop() {
            HasDestination = false;
            IsMoving = false;
            Velocity = { 0.0f, 0.0f };
        }
    };

    struct InventoryComponent {
        struct Item {
            uint32_t ItemID = 0;
            std::string Name;
            uint32_t Quantity = 1;
            std::string IconPath;

            Item() = default;
            Item(uint32_t id, const std::string& name, uint32_t qty = 1)
                : ItemID(id), Name(name), Quantity(qty) {
            }
        };

        std::vector<Item> Items;
        uint32_t MaxSlots = 20;
        uint32_t Gold = 0;

        InventoryComponent() = default;
        InventoryComponent(const InventoryComponent&) = default;
        InventoryComponent(uint32_t maxSlots) : MaxSlots(maxSlots) {}

        bool AddItem(const Item& item) {
            if (Items.size() < MaxSlots) {
                Items.push_back(item);
                return true;
            }
            return false;
        }

        bool RemoveItem(uint32_t itemID, uint32_t quantity = 1) {
            for (auto it = Items.begin(); it != Items.end(); ++it) {
                if (it->ItemID == itemID) {
                    if (it->Quantity > quantity) {
                        it->Quantity -= quantity;
                        return true;
                    }
                    else if (it->Quantity == quantity) {
                        Items.erase(it);
                        return true;
                    }
                    break;
                }
            }
            return false;
        }
    };

    struct NetworkComponent {
        uint32_t NetworkID = 0;
        bool IsLocalPlayer = false;
        bool IsDirty = false; // Needs network update

        // Network sync data
        glm::vec3 LastSyncedPosition = { 0.0f, 0.0f, 0.0f };
        glm::vec3 LastSyncedRotation = { 0.0f, 0.0f, 0.0f };
        float LastSyncTime = 0.0f;

        NetworkComponent() = default;
        NetworkComponent(const NetworkComponent&) = default;
        NetworkComponent(uint32_t id, bool isLocal = false)
            : NetworkID(id), IsLocalPlayer(isLocal) {
        }
    };

    // Audio component
    struct AudioSourceComponent {
        std::string AudioPath;
        bool IsPlaying = false;
        bool IsLooping = false;
        float Volume = 1.0f;
        float Pitch = 1.0f;
        bool Is3D = false;
        float MinDistance = 1.0f;
        float MaxDistance = 100.0f;

        AudioSourceComponent() = default;
        AudioSourceComponent(const AudioSourceComponent&) = default;
        AudioSourceComponent(const std::string& path)
            : AudioPath(path) {
        }
    };

    // Particle system component
    struct ParticleSystemComponent {
        std::string ParticleTexturePath;
        uint32_t MaxParticles = 1000;
        float EmissionRate = 100.0f;
        glm::vec3 StartVelocity = { 0.0f, 5.0f, 0.0f };
        glm::vec3 VelocityVariation = { 2.0f, 1.0f, 2.0f };
        glm::vec4 StartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 EndColor = { 1.0f, 1.0f, 1.0f, 0.0f };
        float StartSize = 1.0f;
        float EndSize = 0.0f;
        float LifeTime = 5.0f;
        float LifeTimeVariation = 1.0f;

        ParticleSystemComponent() = default;
        ParticleSystemComponent(const ParticleSystemComponent&) = default;
    };

} // namespace Cherry
