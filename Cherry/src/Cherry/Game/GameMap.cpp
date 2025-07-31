#include "CHpch.h"
#include "GameMap.h"
namespace Cherry {

    // Map2DItem Implementation
    Map2DItem::Map2DItem(uint32_t itemType, uint32_t itemID)
        : MapObject(MapObjectType::Item2D), m_ItemType(itemType), m_ItemID(itemID) {
        // Load item icon texture based on itemType
        // This would typically load from a texture atlas or individual files
        // For now, use a placeholder
        m_IconTexture = Texture2D::Create("assets/textures/items/item_" + std::to_string(itemType) + ".png");
    }

    void Map2DItem::Show(const glm::mat4& viewMatrix) {
        if (!m_IconTexture) return;

        // Convert cell position to world position
        glm::vec3 worldPos = m_WorldPos;

        // Apply animation (frame cycling)
        float sizeMultiplier = 1.0f + 0.1f * sin(m_AnimationTime * 3.0f); // Subtle pulsing
        glm::vec2 size = { 32.0f * sizeMultiplier, 32.0f * sizeMultiplier };

        // Render the item icon
        BatchRenderer2D::DrawQuad(worldPos, size, m_IconTexture, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

        // Show name if enabled
        if (m_ShowName && !m_ItemName.empty()) {
            // This would render text - for now just a placeholder
            // TextRenderer::DrawString(m_ItemName, worldPos + glm::vec3(0, 40, 0), {1.0f, 1.0f, 0.0f, 1.0f});
        }
    }

    void Map2DItem::Process(float deltaTime) {
        m_AnimationTime += deltaTime;

        // Update frame for animation
        m_FrameIndex = static_cast<uint32_t>(m_AnimationTime * 10.0f) % 4; // 4 frame animation
    }

    bool Map2DItem::IsFocus(const glm::vec2& mousePos) const {
        // Simple bounds check - could be more sophisticated
        glm::vec2 itemScreenPos = { m_WorldPos.x, m_WorldPos.y };
        float radius = 16.0f; // Half the item size

        return glm::distance(mousePos, itemScreenPos) < radius;
    }

    // MapSceneObject Implementation
    MapSceneObject::MapSceneObject(const std::string& aniFile, const std::string& aniIndex)
        : MapObject(MapObjectType::Scene), m_AniFile(aniFile), m_AniIndex(aniIndex) {
        // Load animation texture
        m_Texture = Texture2D::Create("assets/animations/" + aniFile + "/" + aniIndex + ".png");
    }

    void MapSceneObject::Show(const glm::mat4& viewMatrix) {
        if (!m_Texture) return;

        glm::vec3 renderPos = m_WorldPos + glm::vec3(m_Offset.x, m_Offset.y, 0.0f);
        glm::vec2 size = { 64.0f, 64.0f }; // Default size

        BatchRenderer2D::DrawQuad(renderPos, size, m_Texture);
    }

    void MapSceneObject::Process(float deltaTime) {
        m_TimePerFrame += deltaTime;

        if (m_TimePerFrame >= m_FrameInterval) {
            m_FrameIndex++;
            m_TimePerFrame = 0.0f;

            // Reset frame index if needed (depends on animation length)
            if (m_FrameIndex >= 8) { // Assume 8 frame animation
                m_FrameIndex = 0;
            }
        }
    }

    bool MapSceneObject::IsFocus(const glm::vec2& mousePos) const {
        return false; // Scene objects typically aren't interactive
    }

    // InteractiveLayer Implementation
    InteractiveLayer::~InteractiveLayer() {
        ClearMapObjects();
    }

    void InteractiveLayer::Show(const glm::mat4& viewMatrix) {
        for (auto& obj : m_MapObjects) {
            if (obj) {
                obj->Show(viewMatrix);
            }
        }
    }

    void InteractiveLayer::Process(float deltaTime) {
        for (auto& obj : m_MapObjects) {
            if (obj) {
                obj->Process(deltaTime);
            }
        }
    }

    void InteractiveLayer::AddMapObject(std::unique_ptr<MapObject> obj) {
        if (obj) {
            m_MapObjects.push_back(std::move(obj));
        }
    }

    void InteractiveLayer::RemoveMapObject(MapObject* obj) {
        auto it = std::find_if(m_MapObjects.begin(), m_MapObjects.end(),
            [obj](const std::unique_ptr<MapObject>& ptr) {
                return ptr.get() == obj;
            });

        if (it != m_MapObjects.end()) {
            m_MapObjects.erase(it);
        }
    }

    void InteractiveLayer::ClearMapObjects() {
        m_MapObjects.clear();
    }

    MapObject* InteractiveLayer::GetFocusObject(const glm::vec2& mousePos) const {
        for (auto& obj : m_MapObjects) {
            if (obj && obj->IsFocus(mousePos)) {
                return obj.get();
            }
        }
        return nullptr;
    }

    // GameMap Implementation
    bool GameMap::LoadMap(uint32_t mapID) {
        // Unload current map if any
        UnloadMap();

        // Create layers
        m_TerrainLayer = std::make_unique<TerrainLayer>();
        m_InteractiveLayer = std::make_unique<InteractiveLayer>();

        // Load terrain data
        std::string mapFile = "maps/map_" + std::to_string(mapID) + ".dat";
        if (!m_TerrainLayer->LoadMap(mapFile)) {
            CH_CORE_ERROR("Failed to load terrain for map {}", mapID);
            return false;
        }

        m_CurrentMapID = mapID;
        m_MapLoaded = true;

        CH_CORE_INFO("Loaded map {}", mapID);
        return true;
    }

    void GameMap::UnloadMap() {
        m_TerrainLayer.reset();
        m_InteractiveLayer.reset();
        m_MapLoaded = false;
        m_CurrentMapID = 0;
    }

    void GameMap::Cell2World(int cellX, int cellY, float& worldX, float& worldY) const {
        worldX = cellX * m_CellWidth;
        worldY = cellY * m_CellHeight;
    }

    void GameMap::World2Cell(float worldX, float worldY, int& cellX, int& cellY) const {
        cellX = static_cast<int>(worldX / m_CellWidth);
        cellY = static_cast<int>(worldY / m_CellHeight);
    }

    void GameMap::Cell2Screen(int cellX, int cellY, float& screenX, float& screenY, const OrthographicCamera& camera) const {
        float worldX, worldY;
        Cell2World(cellX, cellY, worldX, worldY);
        World2Screen(worldX, worldY, screenX, screenY, camera);
    }

    void GameMap::World2Screen(float worldX, float worldY, float& screenX, float& screenY, const OrthographicCamera& camera) const {
        // Transform world coordinates to screen coordinates using camera
        glm::vec4 worldPos(worldX, worldY, 0.0f, 1.0f);
        glm::vec4 screenPos = camera.GetViewProjectionMatrix() * worldPos;

        screenX = screenPos.x;
        screenY = screenPos.y;
    }

    void GameMap::Screen2World(float screenX, float screenY, float& worldX, float& worldY, const OrthographicCamera& camera) const {
        // Transform screen coordinates to world coordinates
        glm::mat4 invViewProj = glm::inverse(camera.GetViewProjectionMatrix());
        glm::vec4 screenPos(screenX, screenY, 0.0f, 1.0f);
        glm::vec4 worldPos = invViewProj * screenPos;

        worldX = worldPos.x;
        worldY = worldPos.y;
    }

    bool GameMap::IsValidCoordinate(int cellX, int cellY) const {
        if (!m_TerrainLayer) return false;
        return m_TerrainLayer->IsValidCell(cellX, cellY);
    }

    bool GameMap::IsBlocked(int cellX, int cellY) const {
        if (!m_TerrainLayer) return true;
        return m_TerrainLayer->IsBlocked(cellX, cellY);
    }

    void GameMap::AddMapItem(uint32_t itemID, uint32_t itemType, const glm::vec2& cellPos) {
        if (!m_InteractiveLayer) return;

        auto item = std::make_unique<Map2DItem>(itemType, itemID);
        item->SetCellPos(cellPos);

        float worldX, worldY;
        Cell2World(cellPos.x, cellPos.y, worldX, worldY);
        item->SetWorldPos({ worldX, worldY, 0.0f });

        m_InteractiveLayer->AddMapObject(std::move(item));
    }

    void GameMap::AddSceneObject(const std::string& aniFile, const std::string& aniIndex, const glm::vec2& cellPos) {
        if (!m_InteractiveLayer) return;

        auto sceneObj = std::make_unique<MapSceneObject>(aniFile, aniIndex);
        sceneObj->SetCellPos(cellPos);

        float worldX, worldY;
        Cell2World(cellPos.x, cellPos.y, worldX, worldY);
        sceneObj->SetWorldPos({ worldX, worldY, 0.0f });

        m_InteractiveLayer->AddMapObject(std::move(sceneObj));
    }

    void GameMap::AddCoverObject(const std::string& fileName, const std::string& index, const glm::vec2& cellPos) {
        if (!m_InteractiveLayer) return;

        auto coverObj = std::make_unique<MapSceneObject>(fileName, index);
        coverObj->SetCellPos(cellPos);

        float worldX, worldY;
        Cell2World(cellPos.x, cellPos.y, worldX, worldY);
        coverObj->SetWorldPos({ worldX, worldY, 0.0f });

        m_InteractiveLayer->AddMapObject(std::move(coverObj));
    }

    MapObject* GameMap::GetFocusObject(const glm::vec2& mouseScreenPos, const OrthographicCamera& camera) const {
        if (!m_InteractiveLayer) return nullptr;
        return m_InteractiveLayer->GetFocusObject(mouseScreenPos);
    }

    Map2DItem* GameMap::GetMapItem(const glm::vec2& cellPos) const {
        if (!m_InteractiveLayer) return nullptr;
        
        // This is a simplified implementation
        // In a real implementation, you'd have a map of cell positions to items
        return nullptr; // For now, return nullptr
    }

    void GameMap::RemoveMapItem(uint32_t itemID) {
        if (!m_InteractiveLayer) return;
        
        // This is a simplified implementation
        // In a real implementation, you'd find and remove the item by ID
    }

    glm::vec2 GameMap::GetMapSize() const {
        if (!m_TerrainLayer) return { 0.0f, 0.0f };
        glm::ivec2 size = m_TerrainLayer->GetMapSize();
        return { static_cast<float>(size.x), static_cast<float>(size.y) };
    }

    // TerrainLayer implementations
    bool TerrainLayer::LoadMap(const std::string& mapFile) {
        // For now, create a simple procedural map
        // In a real implementation, this would load from a file
        m_MapSize = { 100, 100 }; // 100x100 cell map
        m_Cells.resize(m_MapSize.x * m_MapSize.y);
        
        // Initialize cells with basic terrain
        for (int y = 0; y < m_MapSize.y; y++) {
            for (int x = 0; x < m_MapSize.x; x++) {
                int index = y * m_MapSize.x + x;
                m_Cells[index].Terrain = 1; // Basic grass
                m_Cells[index].Surface = 0;
                m_Cells[index].Height = 0;
                m_Cells[index].Mask = 0; // Walkable
                m_Cells[index].Altitude = 0;
            }
        }
        
        return true;
    }

    void TerrainLayer::Show(const glm::mat4& viewMatrix) {
        // For now, just render a simple grid
        // In a real implementation, this would render terrain textures
        for (int y = 0; y < m_MapSize.y; y++) {
            for (int x = 0; x < m_MapSize.x; x++) {
                float worldX, worldY;
                // Convert cell to world coordinates (simplified)
                worldX = x * 24.0f; // Cell width
                worldY = y * 24.0f; // Cell height
                
                // Render a simple colored quad for each cell
                glm::vec4 color = { 0.2f, 0.8f, 0.2f, 1.0f }; // Green for grass
                BatchRenderer2D::DrawQuad(
                    { worldX, worldY, 0.0f },
                    { 24.0f, 24.0f },
                    color
                );
            }
        }
    }

    void TerrainLayer::Process(float deltaTime) {
        // Terrain processing (animations, etc.)
        // For now, nothing to do
    }

    bool TerrainLayer::IsValidCell(int x, int y) const {
        return x >= 0 && x < m_MapSize.x && y >= 0 && y < m_MapSize.y;
    }

    bool TerrainLayer::IsBlocked(int x, int y) const {
        if (!IsValidCell(x, y)) return true;
        
        int index = y * m_MapSize.x + x;
        return m_Cells[index].IsBlocked();
    }

    MapCell* TerrainLayer::GetCell(int x, int y) {
        if (!IsValidCell(x, y)) return nullptr;
        int index = y * m_MapSize.x + x;
        return &m_Cells[index];
    }

    const MapCell* TerrainLayer::GetCell(int x, int y) const {
        if (!IsValidCell(x, y)) return nullptr;
        int index = y * m_MapSize.x + x;
        return &m_Cells[index];
    }

    void GameMap::Render(const OrthographicCamera& camera) {
        if (!m_MapLoaded) return;

        // Render terrain layer
        if (m_TerrainLayer) {
            m_TerrainLayer->Show(camera.GetViewProjectionMatrix());
        }

        // Render interactive layer
        if (m_InteractiveLayer) {
            m_InteractiveLayer->Show(camera.GetViewProjectionMatrix());
        }

        // Render debug information if enabled
        if (m_DebugMode) {
            RenderDebugGrid(camera);
        }
    }

    void GameMap::Process(float deltaTime) {
        if (!m_MapLoaded) return;

        if (m_TerrainLayer) {
            m_TerrainLayer->Process(deltaTime);
        }

        if (m_InteractiveLayer) {
            m_InteractiveLayer->Process(deltaTime);
        }
    }

    void GameMap::RenderDebugGrid(const OrthographicCamera& camera) {
        if (!m_TerrainLayer) return;

        glm::ivec2 mapSize = m_TerrainLayer->GetMapSize();

        // Draw grid lines
        for (int x = 0; x <= mapSize.x; x++) {
            float worldX1, worldY1, worldX2, worldY2;
            Cell2World(x, 0, worldX1, worldY1);
            Cell2World(x, mapSize.y, worldX2, worldY2);

            BatchRenderer2D::DrawLine({ worldX1, worldY1, 0.1f }, { worldX2, worldY2, 0.1f }, { 0.5f, 0.5f, 0.5f, 0.3f });
        }

        for (int y = 0; y <= mapSize.y; y++) {
            float worldX1, worldY1, worldX2, worldY2;
            Cell2World(0, y, worldX1, worldY1);
            Cell2World(mapSize.x, y, worldX2, worldY2);

            BatchRenderer2D::DrawLine({ worldX1, worldY1, 0.1f }, { worldX2, worldY2, 0.1f }, { 0.5f, 0.5f, 0.5f, 0.3f });
        }
    }

} // namespace Cherry