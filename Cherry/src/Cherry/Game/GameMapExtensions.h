#pragma once
#include "GameMap.h"
#include "Cherry/Core/Threading.h"
#include "Cherry/Renderer/ParticleSystem.h"
#include <future>
#include <Cherry/Network/NetworkManager.h>

namespace Cherry {

    // Advanced map loading with async support
    class MapLoader {
    public:
        static MapLoader& Get() {
            static MapLoader instance;
            return instance;
        }

        // Async map loading
        std::future<bool> LoadMapAsync(uint32_t mapID);

        // Preload multiple maps
        void PreloadMaps(const std::vector<uint32_t>& mapIDs);

        // Map caching
        void SetCacheSize(size_t maxMaps) { m_MaxCachedMaps = maxMaps; }
        void ClearCache();

    private:
        ThreadPool m_LoadingPool{ 2 }; // 2 threads for loading
        std::unordered_map<uint32_t, std::unique_ptr<TerrainLayer>> m_CachedMaps;
        size_t m_MaxCachedMaps = 5;
    };

    // Path finding system (A* implementation)
    class PathFinder {
    public:
        struct PathNode {
            glm::ivec2 Position;
            float GCost = 0.0f;  // Distance from start
            float HCost = 0.0f;  // Distance to end (heuristic)
            float FCost() const { return GCost + HCost; }
            PathNode* Parent = nullptr;
        };

        std::vector<glm::ivec2> FindPath(const glm::ivec2& start, const glm::ivec2& end,
            const GameMap& gameMap);

        void SetDiagonalMovement(bool allow) { m_AllowDiagonal = allow; }
        void SetMaxSearchNodes(int max) { m_MaxSearchNodes = max; }

    private:
        bool m_AllowDiagonal = true;
        int m_MaxSearchNodes = 1000;

        float CalculateHeuristic(const glm::ivec2& a, const glm::ivec2& b);
        std::vector<glm::ivec2> ReconstructPath(PathNode* endNode);
        std::vector<glm::ivec2> GetNeighbors(const glm::ivec2& pos);
    };

    // Optimized rendering with frustum culling
    class MapRenderer {
    public:
        struct RenderStats {
            uint32_t TerrainCellsRendered = 0;
            uint32_t ObjectsRendered = 0;
            uint32_t ObjectsCulled = 0;
            float RenderTime = 0.0f;
        };

        void RenderMap(const GameMap& gameMap, const OrthographicCamera& camera);
        void EnableFrustumCulling(bool enable) { m_FrustumCulling = enable; }
        void SetLODDistance(float distance) { m_LODDistance = distance; }

        const RenderStats& GetStats() const { return m_Stats; }

    private:
        bool m_FrustumCulling = true;
        float m_LODDistance = 500.0f;
        RenderStats m_Stats;

        bool IsInFrustum(const glm::vec3& position, const glm::vec2& size,
            const OrthographicCamera& camera);
    };

    // Sound system integration for map objects
    class MapAudioManager {
    public:
        void PlaySoundAt(const std::string& soundPath, const glm::vec2& position);
        void SetListenerPosition(const glm::vec2& position);
        void UpdateAudio(float deltaTime);

        // Ambient sounds for different terrain types
        void SetTerrainAmbient(uint16_t terrainType, const std::string& ambientSound);

    private:
        glm::vec2 m_ListenerPosition;
        std::unordered_map<uint16_t, std::string> m_TerrainAmbients;
        std::vector<uint32_t> m_ActiveSounds; // Audio handles
    };

    // Map streaming for large worlds
    class MapStreamer {
    public:
        struct StreamingRegion {
            glm::ivec2 Position;
            uint32_t MapID;
            bool IsLoaded = false;
            bool IsVisible = false;
        };

        void Initialize(const glm::ivec2& worldSize, const glm::ivec2& regionSize);
        void SetPlayerPosition(const glm::vec2& position);
        void Update(float deltaTime);

        void SetStreamingDistance(float distance) { m_StreamingDistance = distance; }

    private:
        std::vector<StreamingRegion> m_Regions;
        glm::vec2 m_PlayerPosition;
        float m_StreamingDistance = 500.0f;
        glm::ivec2 m_RegionSize;

        void LoadRegion(StreamingRegion& region);
        void UnloadRegion(StreamingRegion& region);
    };

    // Network synchronization for multiplayer
    class MapNetworkSync {
    public:
        // Object spawning/despawning
        void OnObjectSpawned(uint32_t objectID, MapObjectType type,
            const glm::vec2& position);
        void OnObjectDespawned(uint32_t objectID);
        void OnObjectMoved(uint32_t objectID, const glm::vec2& newPosition);

        // State synchronization
        void SyncMapState();
        void ProcessNetworkUpdate(const NetworkMessage& message);

    private:
        std::unordered_map<uint32_t, MapObject*> m_NetworkObjects;
    };

    // Map editor tools
    class MapEditor {
    public:
        enum class EditMode {
            Terrain,
            Objects,
            Regions,
            Navigation
        };

        void SetEditMode(EditMode mode) { m_CurrentMode = mode; }
        void OnMapClick(const glm::vec2& position, bool rightClick = false);

        // Terrain editing
        void SetTerrainBrush(uint16_t terrainType, uint8_t brushSize);
        void PaintTerrain(const glm::ivec2& center);

        // Object placement
        void PlaceObject(MapObjectType type, const std::string& objectData,
            const glm::vec2& position);
        void RemoveObject(const glm::vec2& position);

        // Save/Load
        void SaveMap(const std::string& filename);
        void LoadMap(const std::string& filename);

    private:
        EditMode m_CurrentMode = EditMode::Terrain;
        uint16_t m_SelectedTerrain = 1;
        uint8_t m_BrushSize = 1;
        MapObjectType m_SelectedObjectType = MapObjectType::Scene;
    };

    // Weather and environment effects
    class WeatherSystem {
    public:
        enum class Weather {
            Clear,
            Rain,
            Snow,
            Fog,
            Storm
        };

        void SetWeather(Weather weather, float intensity = 1.0f);
        void Update(float deltaTime);
        void Render(const OrthographicCamera& camera);

    private:
        Weather m_CurrentWeather = Weather::Clear;
        float m_WeatherIntensity = 1.0f;
        float m_TransitionTime = 0.0f;

        // Particle systems for different weather
        std::unique_ptr<ParticleSystem> m_RainParticles;
        std::unique_ptr<ParticleSystem> m_SnowParticles;
    };

    // Map events and triggers
    class MapEventSystem {
    public:
        struct MapEvent {
            enum Type {
                PlayerEnterRegion,
                PlayerExitRegion,
                ObjectInteraction,
                TimerExpired
            };

            Type EventType;
            glm::vec2 Position;
            uint32_t ObjectID = 0;
            std::string Data;
        };

        using EventCallback = std::function<void(const MapEvent&)>;

        void RegisterEventHandler(MapEvent::Type type, EventCallback callback);
        void TriggerEvent(const MapEvent& event);

        // Region triggers
        void AddRegionTrigger(const glm::vec2& position, float radius,
            const std::string& eventData);

    private:
        std::unordered_map<MapEvent::Type, std::vector<EventCallback>> m_EventHandlers;

        struct RegionTrigger {
            glm::vec2 Position;
            float Radius;
            std::string Data;
            bool HasTriggered = false;
        };
        std::vector<RegionTrigger> m_RegionTriggers;
    };

} // namespace Cherry