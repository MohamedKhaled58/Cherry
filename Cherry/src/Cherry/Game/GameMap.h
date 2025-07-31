#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Renderer/Texture.h"
#include "Cherry/Renderer/BatchRenderer2D.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Scene/Entity.h"
#include "Cherry/Scene/Components.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Cherry {

    // Map cell structure similar to original CLayer
    struct MapCell {
        uint16_t Terrain = 0;      // Ground texture ID
        uint16_t Surface = 0;      // Surface object ID  
        uint8_t Height = 0;        // Height level
        uint8_t Mask = 0;          // Walkability mask
        int16_t Altitude = 0;      // Similar to original sAltitude

        bool IsBlocked() const { return (Mask & 0x01) != 0; }
        bool CanShoot() const { return (Mask & 0x02) != 0; }
    };

    // Map object types (from original)
    enum class MapObjectType {
        None = 0,
        Terrain = 1,
        TerrainPart = 2,
        Scene = 3,
        Cover = 4,
        Role = 5,
        Hero = 6,
        Player = 7,
        Puzzle = 8,
        Item2D = 11,
        Item3D = 18,
        Effect3D = 10,
        Sound = 15,
        Region = 16
    };

    // Base map object (inspired by original CMapObj)
    class MapObject {
    public:
        MapObject(MapObjectType type) : m_Type(type) {}
        virtual ~MapObject() = default;

        virtual void Show(const glm::mat4& viewMatrix) = 0;
        virtual void Process(float deltaTime) = 0;
        virtual bool IsFocus(const glm::vec2& mousePos) const = 0;

        MapObjectType GetType() const { return m_Type; }

        const glm::vec2& GetCellPos() const { return m_CellPos; }
        void SetCellPos(const glm::vec2& pos) { m_CellPos = pos; }

        const glm::vec3& GetWorldPos() const { return m_WorldPos; }
        void SetWorldPos(const glm::vec3& pos) { m_WorldPos = pos; }

    protected:
        MapObjectType m_Type;
        glm::vec2 m_CellPos = { 0, 0 };
        glm::vec3 m_WorldPos = { 0, 0, 0 };
    };

    // 2D Map Item (similar to original C2DMapItem)
    class Map2DItem : public MapObject {
    public:
        Map2DItem(uint32_t itemType, uint32_t itemID);

        void Show(const glm::mat4& viewMatrix) override;
        void Process(float deltaTime) override;
        bool IsFocus(const glm::vec2& mousePos) const override;

        uint32_t GetItemType() const { return m_ItemType; }
        uint32_t GetItemID() const { return m_ItemID; }

        void SetShowName(bool show, const std::string& name = "");

    private:
        uint32_t m_ItemType;
        uint32_t m_ItemID;
        uint32_t m_FrameIndex = 0;
        bool m_ShowName = false;
        std::string m_ItemName;
        REF(Texture2D) m_IconTexture;
        float m_AnimationTime = 0.0f;
    };

    // Scene Object (similar to original C2DMapSceneObj)
    class MapSceneObject : public MapObject {
    public:
        MapSceneObject(const std::string& aniFile, const std::string& aniIndex);

        void Show(const glm::mat4& viewMatrix) override;
        void Process(float deltaTime) override;
        bool IsFocus(const glm::vec2& mousePos) const override;

        void SetFrameInterval(float interval) { m_FrameInterval = interval; }
        void SetOffset(const glm::vec2& offset) { m_Offset = offset; }

    private:
        std::string m_AniFile;
        std::string m_AniIndex;
        glm::vec2 m_Offset = { 0, 0 };
        uint32_t m_FrameIndex = 0;
        float m_FrameInterval = 1.0f / 60.0f;
        float m_TimePerFrame = 0.0f;
        REF(Texture2D) m_Texture;
    };

    // Cover Object (similar to original C2DMapCoverObj)  
    class MapCoverObject : public MapObject {
    public:
        MapCoverObject(const std::string& fileName, const std::string& index);

        void Show(const glm::mat4& viewMatrix) override;
        void Process(float deltaTime) override;
        bool IsFocus(const glm::vec2& mousePos) const override;

    private:
        std::string m_FileName;
        std::string m_Index;
        REF(Texture2D) m_Texture;
        glm::vec2 m_BaseSize = { 1.0f, 1.0f };
    };

    // Interactive Layer (similar to original CInteractiveLayer)
    class InteractiveLayer {
    public:
        InteractiveLayer() = default;
        ~InteractiveLayer();

        void Show(const glm::mat4& viewMatrix);
        void Process(float deltaTime);

        void AddMapObject(std::unique_ptr<MapObject> obj);
        void RemoveMapObject(MapObject* obj);
        void ClearMapObjects();

        MapObject* GetFocusObject(const glm::vec2& mousePos) const;
        std::vector<MapObject*> GetObjectsInRange(const glm::vec2& center, float range) const;

    private:
        std::vector<std::unique_ptr<MapObject>> m_MapObjects;
    };

    // Terrain Layer (similar to original CTerrainLayer)
    class TerrainLayer {
    public:
        TerrainLayer() = default;
        ~TerrainLayer() = default;

        bool LoadMap(const std::string& mapFile);
        void Show(const glm::mat4& viewMatrix);
        void Process(float deltaTime);

        bool IsValidCell(int x, int y) const;
        bool IsBlocked(int x, int y) const;
        MapCell* GetCell(int x, int y);
        const MapCell* GetCell(int x, int y) const;

        glm::ivec2 GetMapSize() const { return m_MapSize; }

    private:
        std::vector<MapCell> m_Cells;
        glm::ivec2 m_MapSize = { 0, 0 };
        REF(Texture2D) m_TerrainTexture;
    };

    // Main GameMap class (inspired by original C3DGameMap)
    class GameMap {
    public:
        static GameMap& Get() {
            static GameMap instance;
            return instance;
        }

        // Map loading and management
        bool LoadMap(uint32_t mapID);
        void UnloadMap();
        bool IsMapLoaded() const { return m_MapLoaded; }
        uint32_t GetCurrentMapID() const { return m_CurrentMapID; }

        // Coordinate conversion (similar to original)
        void Cell2World(int cellX, int cellY, float& worldX, float& worldY) const;
        void World2Cell(float worldX, float worldY, int& cellX, int& cellY) const;
        void Cell2Screen(int cellX, int cellY, float& screenX, float& screenY, const OrthographicCamera& camera) const;
        void Screen2Cell(float screenX, float screenY, int& cellX, int& cellY, const OrthographicCamera& camera) const;
        void World2Screen(float worldX, float worldY, float& screenX, float& screenY, const OrthographicCamera& camera) const;
        void Screen2World(float screenX, float screenY, float& worldX, float& worldY, const OrthographicCamera& camera) const;

        // Map queries
        bool IsValidCoordinate(int cellX, int cellY) const;
        bool IsBlocked(int cellX, int cellY) const;
        bool IsPosVisible(const glm::vec2& cellPos, const OrthographicCamera& camera) const;

        // Object management
        void AddMapItem(uint32_t itemID, uint32_t itemType, const glm::vec2& cellPos);
        void RemoveMapItem(uint32_t itemID);
        Map2DItem* GetMapItem(const glm::vec2& cellPos) const;

        void AddSceneObject(const std::string& aniFile, const std::string& aniIndex, const glm::vec2& cellPos);
        void AddCoverObject(const std::string& fileName, const std::string& index, const glm::vec2& cellPos);

        // Focus and interaction
        MapObject* GetFocusObject(const glm::vec2& mouseScreenPos, const OrthographicCamera& camera) const;
        std::vector<uint32_t> GetPlayersInRange(const glm::vec2& center, float range) const;

        // Rendering
        void Render(const OrthographicCamera& camera);
        void Process(float deltaTime);

        // Configuration
        void SetScale(float scale) { m_Scale = scale; }
        float GetScale() const { return m_Scale; }
        void SetCellSize(float cellWidth, float cellHeight) {
            m_CellWidth = cellWidth;
            m_CellHeight = cellHeight;
        }

        // Utility
        glm::vec2 GetMapSize() const;
        void EnableDebugMode(bool enable) { m_DebugMode = enable; }
        bool IsDebugMode() const { return m_DebugMode; }

    private:
        GameMap() = default;
        ~GameMap() = default;

        // Map data
        bool m_MapLoaded = false;
        uint32_t m_CurrentMapID = 0;

        // Layers
        std::unique_ptr<TerrainLayer> m_TerrainLayer;
        std::unique_ptr<InteractiveLayer> m_InteractiveLayer;

        // Map configuration (similar to original constants)
        float m_CellWidth = 24.0f;   // _CELL_WIDTH equivalent
        float m_CellHeight = 24.0f;  // _CELL_HEIGHT equivalent
        float m_Scale = 1.0f;

        // Debug
        bool m_DebugMode = false;

        // Rendering helpers
        void RenderDebugGrid(const OrthographicCamera& camera);
        void RenderDebugInfo(const OrthographicCamera& camera);
    };

} // namespace Cherry