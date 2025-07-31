#include "CHpch.h"
#include "Entity.h"

namespace Cherry {

    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene) {
    }

} // namespace Cherry