#pragma once
#include <glm/glm.hpp>
#include <Cherry/Renderer/Texture.h>
#include <Cherry/Renderer/Sprite2D.h>
namespace Cherry {
    struct MapCell {
        uint16_t Terrain;      // Ground texture ID
        uint16_t Surface;      // Surface object ID  
        uint8_t Height;        // Height level
        uint8_t Mask;          // Walkability mask

        bool IsBlocked() const { return (Mask & 0x01) != 0; }
        bool CanShoot() const { return (Mask & 0x02) != 0; }
    };

    class GameMap {
    public:
        bool LoadMap(uint32_t mapID);
        bool IsValidCoordinate(uint16_t x, uint16_t y) const;
        bool IsBlocked(uint16_t x, uint16_t y) const;

        // Path finding for NPCs/Monsters
        std::vector<glm::ivec2> FindPath(glm::ivec2 start, glm::ivec2 end);

        // Get visible area (for networking optimization)
        std::vector<uint32_t> GetPlayersInRange(uint16_t x, uint16_t y, uint16_t range);

        void Render(const glm::mat4& viewMatrix);

    private:
        uint32_t m_MapID;
        uint32_t m_Width, m_Height;
        std::vector<MapCell> m_Cells;

        // Rendering data
        Ref<Texture2D> m_TerrainTextures;
        std::vector<Ref<Sprite2D>> m_StaticObjects;
    };
}