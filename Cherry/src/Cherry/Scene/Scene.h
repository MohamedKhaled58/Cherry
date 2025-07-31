#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Core/TimeStep.h"
#include "Cherry/Renderer/Camera.h"
#include <entt.hpp>
#include <string>

namespace Cherry {

    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        // Entity management
        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(const std::string& name = std::string());
        void DestroyEntity(Entity entity);
        void DuplicateEntity(Entity entity);

        // Entity queries
        Entity FindEntityByName(const std::string& name);
        Entity GetEntityByUUID(uint64_t uuid);

        // Scene lifecycle
        void OnUpdateRuntime(TimeStep ts);
        void OnUpdateEditor(TimeStep ts, OrthographicCamera& camera);
        void OnViewportResize(uint32_t width, uint32_t height);

        // Primary camera
        Entity GetPrimaryCameraEntity();

        // Scene serialization
        void Serialize(const std::string& filepath);
        bool Deserialize(const std::string& filepath);

        // Template methods for component handling
        template<typename... Components>
        auto GetAllEntitiesWith() {
            return m_Registry.view<Components...>();
        }

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        template<typename T>

        void OnPhysics2DStart();
        void OnPhysics2DStop();

        void RenderScene(OrthographicCamera& camera);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        // Physics world
        void* m_PhysicsWorld = nullptr;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };


} // namespace Cherry
