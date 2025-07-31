#pragma once
#include "CHpch.h"
#include "../entt/entt.hpp"
#include "Cherry/Core/Core.h"
#include "Cherry/Core/TimeStep.h"
#include "Cherry/Renderer/Camera.h"
#include <string>
#include "EditorCamera.h"

namespace Cherry {

    class Entity;
    class EditorCamera;
    class ScriptableEntity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(const std::string& name = std::string());
        void DestroyEntity(Entity entity);
        void DuplicateEntity(Entity entity);

        Entity FindEntityByName(const std::string& name);
        Entity GetEntityByUUID(uint64_t uuid);

        void OnUpdateEditor(TimeStep ts, EditorCamera& camera);
        void OnUpdateRuntime(TimeStep ts);
        void OnViewportResize(uint32_t width, uint32_t height);

        Entity GetPrimaryCameraEntity();

        void Serialize(const std::string& filepath);
        bool Deserialize(const std::string& filepath);

        template<typename... Components>
        auto GetAllEntitiesWith() {
            return m_Registry.view<Components...>();
        }

        template<typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        void OnPhysics2DStart();
        void OnPhysics2DStop();

        void RenderScene(EditorCamera& camera);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        void* m_PhysicsWorld = nullptr;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };


} // namespace Cherry