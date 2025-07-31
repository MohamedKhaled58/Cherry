#include <glm/glm.hpp>
#include "Cherry/Core/Core.h"
namespace Cherry {
    struct SpriteVertex {
        glm::vec3 Position;
        float RHW = 1.0f;
        glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec2 UV;
    };

    class Sprite2D {
    public:
        // C3-style vertex manipulation
        void SetVertexColor(uint32_t ltColor, uint32_t rtColor,
            uint32_t lbColor, uint32_t rbColor);
        void SetCoordinates(const Rect* srcRect, int x, int y,
            uint32_t width = 0, uint32_t height = 0);
        void Mirror();

        // Blending modes from C3Engine
        enum BlendMode {
            Normal = 0,     // Standard alpha blend
            Additive = 1,   // Add colors
            Multiply = 2    // Multiply colors
        };

        bool Draw(BlendMode blendMode = Normal);

        // Multi-texture blending (like C3Engine dual-texture)
        bool DrawBlended(Ref<Sprite2D> bottomSprite,
            uint8_t alphaA, uint8_t alphaB,
            uint8_t alphaC, uint8_t alphaD);

    private:
        SpriteVertex m_Vertices[4];
        Ref<Texture2D> m_Texture;
    };
}