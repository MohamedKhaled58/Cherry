
// Cherry/src/Cherry/Network/NetworkedEntity.h
#pragma once
#include "Cherry/Scene/Entity.h"
#include "Cherry/Scene/Components.h"
#include "NetworkManager.h"

namespace Cherry {

    class NetworkedEntityManager {
    public:
        static NetworkedEntityManager& Get() {
            static NetworkedEntityManager instance;
            return instance;
        }

        // Entity synchronization
        void RegisterNetworkedEntity(Entity entity);
        void UnregisterNetworkedEntity(Entity entity);
        void SynchronizeEntity(Entity entity);
        void SynchronizeAllEntities();

        // Network event handling
        void OnEntitySpawned(uint32_t networkID, const std::string& entityType, const glm::vec3& position);
        void OnEntityDespawned(uint32_t networkID);
        void OnEntityUpdated(uint32_t networkID, const glm::vec3& position, const glm::vec3& rotation);

        // Configuration
        void SetSyncRate(float rate) { m_SyncRate = rate; }
        void SetInterpolationEnabled(bool enabled) { m_InterpolationEnabled = enabled; }

    private:
        std::unordered_map<uint32_t, Entity> m_NetworkedEntities;
        float m_SyncRate = 20.0f; // Hz
        float m_LastSyncTime = 0.0f;
        bool m_InterpolationEnabled = true;

        // Position interpolation data
        struct InterpolationData {
            glm::vec3 StartPosition;
            glm::vec3 TargetPosition;
            glm::vec3 StartRotation;
            glm::vec3 TargetRotation;
            float StartTime;
            float Duration;
        };
        std::unordered_map<uint32_t, InterpolationData> m_InterpolationData;
    };

} // namespace Cherry