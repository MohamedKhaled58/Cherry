#pragma once
#include "CHpch.h"
#include "Cherry/Core/Core.h"
#include "Cherry/Renderer/Texture.h"
#include "Cherry/Renderer/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <unordered_map>
#include "ScriptableEntity.h"


namespace Cherry {

    // Forward declarations
    class ScriptableEntity;
    struct AnimationClip;

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
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    // Fixed NativeScriptComponent
    struct NativeScriptComponent {
        ScriptableEntity* Instance = nullptr;

        std::function<ScriptableEntity* ()> InstantiateScript;
        std::function<void(NativeScriptComponent*)> DestroyScript;

        template<typename T>
        void Bind() {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) {
                delete nsc->Instance;
                nsc->Instance = nullptr;
                };
        }
    };

    // Physics components
    struct RigidBody2DComponent {
        enum class BodyType { Static = 0, Kinematic, Dynamic };
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        void* RuntimeBody = nullptr;

        RigidBody2DComponent() = default;
        RigidBody2DComponent(const RigidBody2DComponent&) = default;
    };

    struct BoxCollider2DComponent {
        glm::vec2 Offset = { 0.0f, 0.0f };
        glm::vec2 Size = { 0.5f, 0.5f };

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

    struct CircleCollider2DComponent {
        glm::vec2 Offset = { 0.0f, 0.0f };
        float Radius = 0.5f;

        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    // Simple AnimationClip definition to fix compilation
    struct AnimationClip {
        std::string Name;
        float Duration = 1.0f;
        bool IsLooping = true;

        AnimationClip() = default;
        AnimationClip(const std::string& name, float duration = 1.0f)
            : Name(name), Duration(duration) {
        }
    };

    // Fixed AnimationComponent
    struct AnimationComponent {
        std::string CurrentAnimation;
        float AnimationTime = 0.0f;
        float AnimationSpeed = 1.0f;
        bool IsPlaying = false;
        bool IsLooping = true;

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

    // Additional game components...
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
        std::string NPCType;
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

    struct NetworkComponent {
        uint32_t NetworkID = 0;
        bool IsLocalPlayer = false;
        bool IsDirty = false;

        glm::vec3 LastSyncedPosition = { 0.0f, 0.0f, 0.0f };
        glm::vec3 LastSyncedRotation = { 0.0f, 0.0f, 0.0f };
        float LastSyncTime = 0.0f;

        NetworkComponent() = default;
        NetworkComponent(const NetworkComponent&) = default;
        NetworkComponent(uint32_t id, bool isLocal = false)
            : NetworkID(id), IsLocalPlayer(isLocal) {
        }
    };

    struct AudioSourceComponent {
        std::string ClipPath;
        uint32_t Handle = 0; // INVALID_AUDIO_HANDLE
        float Volume = 1.0f;
        float Pitch = 1.0f;
        bool IsLooping = false;
        bool PlayOnAwake = false;
        bool Is3D = true;
        std::string MixerChannel = "SFX";

        // 3D settings
        float MinDistance = 1.0f;
        float MaxDistance = 100.0f;
        float RolloffFactor = 1.0f;

        AudioSourceComponent() = default;
        AudioSourceComponent(const AudioSourceComponent&) = default;
        AudioSourceComponent(const std::string& clipPath) : ClipPath(clipPath) {}
    };

    struct InventoryComponent {
        std::unordered_map<uint32_t, uint32_t> Items; // ItemID -> Quantity
        uint32_t MaxSlots = 20;
        uint32_t UsedSlots = 0;

        InventoryComponent() = default;
        InventoryComponent(const InventoryComponent&) = default;
        InventoryComponent(uint32_t maxSlots) : MaxSlots(maxSlots) {}

        bool AddItem(uint32_t itemID, uint32_t quantity = 1) {
            if (UsedSlots >= MaxSlots) return false;
            Items[itemID] += quantity;
            UsedSlots++;
            return true;
        }

        bool RemoveItem(uint32_t itemID, uint32_t quantity = 1) {
            auto it = Items.find(itemID);
            if (it == Items.end() || it->second < quantity) return false;
            it->second -= quantity;
            if (it->second == 0) {
                Items.erase(it);
                UsedSlots--;
            }
            return true;
        }
    };

    struct ParticleSystemComponent {
        std::string ParticleEffect;
        bool IsActive = false;
        float EmissionRate = 10.0f;
        float Lifetime = 2.0f;
        glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        float Size = 1.0f;
        glm::vec2 Velocity = { 0.0f, 0.0f };
        float VelocityVariation = 0.5f;

        ParticleSystemComponent() = default;
        ParticleSystemComponent(const ParticleSystemComponent&) = default;
        ParticleSystemComponent(const std::string& effect)
            : ParticleEffect(effect) {
        }

        void Start() { IsActive = true; }
        void Stop() { IsActive = false; }
    };

} // namespace Cherry