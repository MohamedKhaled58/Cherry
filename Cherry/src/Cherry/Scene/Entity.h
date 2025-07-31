#pragma once
#include "Cherry/Core/Core.h"
#include "Scene.h"
#include <../entt/entt.hpp>
#include <functional> // for std::hash

namespace Cherry {

    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene);
        Entity(const Entity& other) = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args) {
            CH_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args) {
            T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T>
        T& GetComponent() {
            CH_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() {
            return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent() {
            CH_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return (uint32_t)m_EntityHandle; }

        bool operator==(const Entity& other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const {
            return !(*this == other);
        }

        // Accessor methods for hash function
        entt::entity GetEntityHandle() const { return m_EntityHandle; }
        Scene* GetScene() const { return m_Scene; }

    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;
    };

} // namespace Cherry

// Hash specialization for Cherry::Entity to enable use in std::unordered_map
namespace std {
    template<>
    struct hash<Cherry::Entity> {
        size_t operator()(const Cherry::Entity& entity) const noexcept {
            // Combine hash of entity handle and scene pointer
            size_t h1 = hash<entt::entity>{}(entity.GetEntityHandle());
            size_t h2 = hash<Cherry::Scene*>{}(entity.GetScene());
            return h1 ^ (h2 << 1); // Simple hash combination
        }
    };
}
