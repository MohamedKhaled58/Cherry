// Cherry/src/Cherry/Renderer/BatchRenderer2D.h
#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/Texture.h"
#include "Cherry/Renderer/Shader.h"
#include "Cherry/Renderer/VertexArray.h"
#include <glm/glm.hpp>
#include <array>

namespace Cherry {

    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;

        // Additional properties for advanced rendering
        glm::vec2 Normal;           // For lighting effects
        float Roughness = 1.0f;     // Material property
        float Metallic = 0.0f;      // Material property
    };

    struct RenderStatistics {
        uint32_t DrawCalls = 0;
        uint32_t QuadCount = 0;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
        uint32_t TextureSlots = 0;

        void Reset() {
            DrawCalls = QuadCount = VertexCount = IndexCount = TextureSlots = 0;
        }
    };

    struct RenderLimits {
        static constexpr uint32_t MaxQuads = 20000;
        static constexpr uint32_t MaxVertices = MaxQuads * 4;
        static constexpr uint32_t MaxIndices = MaxQuads * 6;
        static constexpr uint32_t MaxTextureSlots = 32;
    };

    class BatchRenderer2D {
    public:
        static void Init();
        static void Shutdown();

        // Scene management
        static void BeginScene(const OrthographicCamera& camera);
        static void BeginScene(const glm::mat4& viewProjectionMatrix);
        static void EndScene();

        // Batch management
        static void StartBatch();
        static void Flush();
        static void NextBatch();

        // Drawing primitives
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, REF(Texture2D) texture,
            float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, REF(Texture2D) texture,
            float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        // Rotated quads
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
            REF(Texture2D) texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
            REF(Texture2D) texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        // Advanced drawing with full transform matrix
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
        static void DrawQuad(const glm::mat4& transform, REF(Texture2D) texture,
            float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

        // Sprite drawing with texture atlas support
        static void DrawSprite(const glm::mat4& transform, const glm::vec4& color,
            const glm::vec2& texCoordMin, const glm::vec2& texCoordMax,
            REF(Texture2D) texture, float tilingFactor = 1.0f, int entityID = -1);

        // Line drawing for debug and UI
        static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
        static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawCircle(const glm::vec3& position, float radius, const glm::vec4& color, float thickness = 0.1f, float fade = 0.005f);

        // Text rendering (requires font atlas)
        //static void DrawString(const std::string& string, REF(Font) font, const glm::mat4& transform,const glm::vec4& color, int entityID = -1);

        // Statistics
        static const RenderStatistics& GetStats() { return s_Data.Stats; }
        static void ResetStats() { s_Data.Stats.Reset(); }

        // Configuration
        static void SetLineWidth(float width);
        static void EnableWireframe(bool enable);

    private:
        struct RendererData {
            // Quad rendering
            REF(VertexArray) QuadVertexArray;
            REF(VertexBuffer) QuadVertexBuffer;
            REF(Shader) QuadShader;
            REF(Texture2D) WhiteTexture;

            // Line rendering
            REF(VertexArray) LineVertexArray;
            REF(VertexBuffer) LineVertexBuffer;
            REF(Shader) LineShader;

            // Circle rendering
            REF(VertexArray) CircleVertexArray;
            REF(VertexBuffer) CircleVertexBuffer;
            REF(Shader) CircleShader;

            // Text rendering
            REF(Shader) TextShader;

            // Batch data
            uint32_t QuadIndexCount = 0;
            QuadVertex* QuadVertexBufferBase = nullptr;
            QuadVertex* QuadVertexBufferPtr = nullptr;

            uint32_t LineVertexCount = 0;
            QuadVertex* LineVertexBufferBase = nullptr;
            QuadVertex* LineVertexBufferPtr = nullptr;

            uint32_t CircleIndexCount = 0;
            QuadVertex* CircleVertexBufferBase = nullptr;
            QuadVertex* CircleVertexBufferPtr = nullptr;

            // Texture management
            std::array<REF(Texture2D), RenderLimits::MaxTextureSlots> TextureSlots;
            uint32_t TextureSlotIndex = 1; // 0 = white texture

            // Shader uniforms
            glm::mat4 ViewProjectionMatrix;

            // Statistics
            RenderStatistics Stats;

            // Configuration
            float LineWidth = 2.0f;
            bool WireframeMode = false;

            // Vertex positions for quads (cached for performance)
            glm::vec4 QuadVertexPositions[4];
        };

        static RendererData s_Data;

        // Helper functions
        static float GetTextureIndex(REF(Texture2D) texture);
        static void FlushAndReset();
        static void SetupQuadVertices(const glm::mat4& transform, const glm::vec4& color,
            const std::array<glm::vec2, 4>& textureCoords,
            float textureIndex, float tilingFactor);
    };

    // Implementation details
    inline void BatchRenderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    inline void BatchRenderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, color);
    }

    inline void BatchRenderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
        REF(Texture2D) texture, float tilingFactor, const glm::vec4& tintColor) {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    inline void BatchRenderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size,
        REF(Texture2D) texture, float tilingFactor, const glm::vec4& tintColor) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

    inline void BatchRenderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
        float rotation, const glm::vec4& color) {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
    }

    inline void BatchRenderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size,
        float rotation, const glm::vec4& color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, color);
    }

    inline void BatchRenderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size,
        float rotation, REF(Texture2D) texture,
        float tilingFactor, const glm::vec4& tintColor) {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
    }

    inline void BatchRenderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size,
        float rotation, REF(Texture2D) texture,
        float tilingFactor, const glm::vec4& tintColor) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, texture, tilingFactor, tintColor);
    }

} // namespace Cherry
