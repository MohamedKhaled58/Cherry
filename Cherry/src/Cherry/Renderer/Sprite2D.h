// Cherry/src/Cherry/Renderer/EnhancedSprite2D.h
#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Renderer/Texture.h"
#include "Cherry/Renderer/BatchRenderer2D.h"
#include <glm/glm.hpp>
#include <array>

namespace Cherry {

    // Enhanced sprite vertex with all C3Engine features
    struct EnhancedSpriteVertex {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float RHW = 1.0f;
        glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec2 UV = { 0.0f, 0.0f };

        // Additional properties for advanced effects
        glm::vec2 Normal = { 0.0f, 0.0f };
        float Metallic = 0.0f;
        float Roughness = 1.0f;
    };

    // Blending modes from C3Engine
    enum class SpriteBlendMode {
        Normal = 0,     // Standard alpha blend
        Additive = 1,   // Add colors
        Multiply = 2,   // Multiply colors
        Screen = 3,     // Screen blend
        Overlay = 4,    // Overlay blend
        SoftLight = 5,  // Soft light
        HardLight = 6   // Hard light
    };

    // Advanced sprite class with C3Engine compatibility
    class EnhancedSprite2D {
    public:
        EnhancedSprite2D();
        explicit EnhancedSprite2D(REF(Texture2D) texture);
        ~EnhancedSprite2D() = default;

        // C3Engine compatible interface
        void SetTexture(REF(Texture2D) texture) { m_Texture = texture; }
        REF(Texture2D) GetTexture() const { return m_Texture; }

        // Vertex manipulation (C3Engine style)
        void SetVertexColor(uint32_t ltColor, uint32_t rtColor, uint32_t lbColor, uint32_t rbColor);
        void SetColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
        void SetColor(const glm::vec4& color);

        // Coordinate setup (C3Engine style)
        void SetCoordinates(const glm::ivec4* srcRect, int x, int y, uint32_t width = 0, uint32_t height = 0);
        void SetCoordinates(int srcX, int srcY, int srcW, int srcH, int x, int y, uint32_t width = 0, uint32_t height = 0);

        // UV manipulation
        void SetUVRect(float u1, float v1, float u2, float v2);
        void SetUVRect(const glm::vec4& uvRect);
        glm::vec4 GetUVRect() const;

        // Transform operations
        void SetPosition(const glm::vec2& position);
        void SetPosition(float x, float y);
        glm::vec2 GetPosition() const;

        void SetSize(const glm::vec2& size);
        void SetSize(float width, float height);
        glm::vec2 GetSize() const;

        void SetRotation(float rotation);
        float GetRotation() const { return m_Rotation; }

        void SetScale(const glm::vec2& scale);
        void SetScale(float scale);
        glm::vec2 GetScale() const { return m_Scale; }

        void SetOrigin(const glm::vec2& origin);
        glm::vec2 GetOrigin() const { return m_Origin; }

        // Effects
        void Mirror();
        void FlipVertical();
        void FlipHorizontal();

        // Advanced rendering features
        void SetBlendMode(SpriteBlendMode blendMode) { m_BlendMode = blendMode; }
        SpriteBlendMode GetBlendMode() const { return m_BlendMode; }

        void SetOpacity(float opacity) { m_Opacity = glm::clamp(opacity, 0.0f, 1.0f); }
        float GetOpacity() const { return m_Opacity; }

        // Multi-texture blending (C3Engine dual-texture support)
        bool DrawBlended(REF(EnhancedSprite2D) bottomSprite,
            uint8_t alphaA, uint8_t alphaB, uint8_t alphaC, uint8_t alphaD);

        // Rendering
        bool Draw(SpriteBlendMode blendMode = SpriteBlendMode::Normal);
        void DrawImmediate(); // For direct rendering without batching

        // Batch rendering support
        void SubmitToBatch(class SpriteBatch& batch);

        // Animation support
        struct AnimationFrame {
            glm::vec4 UVRect;
            float Duration;
            std::string Name;
        };

        void AddAnimationFrame(const std::string& name, const glm::vec4& uvRect, float duration);
        void PlayAnimation(const std::string& name, bool loop = true);
        void StopAnimation();
        void UpdateAnimation(float deltaTime);
        bool IsAnimationPlaying() const { return m_IsAnimating; }

        // Utility functions
        glm::vec2 GetTextureSize() const;
        glm::mat4 GetTransformMatrix() const;

        // Bounds checking
        glm::vec4 GetBounds() const; // Returns (x, y, width, height)
        bool Contains(const glm::vec2& point) const;
        bool Intersects(const EnhancedSprite2D& other) const;

        // Vertex access (for advanced usage)
        std::array<EnhancedSpriteVertex, 4>& GetVertices() { return m_Vertices; }
        const std::array<EnhancedSpriteVertex, 4>& GetVertices() const { return m_Vertices; }

    private:
        REF(Texture2D) m_Texture;
        std::array<EnhancedSpriteVertex, 4> m_Vertices;

        // Transform properties
        glm::vec2 m_Position = { 0.0f, 0.0f };
        glm::vec2 m_Size = { 0.0f, 0.0f };
        glm::vec2 m_Scale = { 1.0f, 1.0f };
        glm::vec2 m_Origin = { 0.0f, 0.0f }; // Pivot point (0,0 = top-left, 0.5,0.5 = center)
        float m_Rotation = 0.0f;
        float m_Opacity = 1.0f;

        // Rendering properties
        SpriteBlendMode m_BlendMode = SpriteBlendMode::Normal;

        // Animation
        struct AnimationData {
            std::vector<AnimationFrame> Frames;
            size_t CurrentFrame = 0;
            float CurrentTime = 0.0f;
            bool IsLooping = false;
            bool IsActive = false;
        };
        std::unordered_map<std::string, AnimationData> m_Animations;
        std::string m_CurrentAnimation;
        bool m_IsAnimating = false;

        void UpdateVertices();
        void SetupDefaultVertices();
        glm::vec4 ColorToVec4(uint32_t color) const;
        uint32_t Vec4ToColor(const glm::vec4& color) const;
    };

    // Sprite batch renderer for efficient rendering of many sprites
    class SpriteBatch {
    public:
        explicit SpriteBatch(size_t maxSprites = 10000);
        ~SpriteBatch();

        void Begin(const glm::mat4& viewProjectionMatrix);
        void End();

        void Draw(REF(EnhancedSprite2D) sprite);
        void Draw(REF(Texture2D) texture, const glm::vec2& position, const glm::vec4& color = { 1,1,1,1 });
        void Draw(REF(Texture2D) texture, const glm::vec2& position, const glm::vec2& size,
            const glm::vec4& uvRect, const glm::vec4& color = { 1,1,1,1 });

        // Advanced drawing with full control
        void DrawQuad(REF(Texture2D) texture, const std::array<EnhancedSpriteVertex, 4>& vertices);

        void Flush();

        // Statistics
        struct Stats {
            size_t SpritesDrawn = 0;
            size_t DrawCalls = 0;
            size_t VerticesSubmitted = 0;
            size_t TextureBinds = 0;
        };

        Stats GetStats() const { return m_Stats; }
        void ResetStats() { m_Stats = {}; }

    private:
        struct BatchVertex : public EnhancedSpriteVertex {
            float TextureIndex = 0.0f;
        };

        std::vector<BatchVertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::array<REF(Texture2D), 32> m_Textures; // Max 32 textures per batch

        size_t m_MaxSprites;
        size_t m_CurrentSprite = 0;
        size_t m_CurrentTexture = 0;

        // OpenGL resources
        uint32_t m_VAO = 0;
        uint32_t m_VBO = 0;
        uint32_t m_EBO = 0;
        REF(class Shader) m_Shader;

        glm::mat4 m_ViewProjectionMatrix;
        Stats m_Stats;
        bool m_HasBegun = false;

        void Initialize();
        void Shutdown();
        float GetTextureIndex(REF(Texture2D) texture);
        void SetupBlendMode(SpriteBlendMode blendMode);
    };

    // Sprite manager for automatic batching and resource management
    class SpriteManager {
    public:
        static SpriteManager& Get() {
            static SpriteManager instance;
            return instance;
        }

        void Initialize();
        void Shutdown();

        // Factory methods
        REF(EnhancedSprite2D) CreateSprite(const std::string& texturePath);
        REF(EnhancedSprite2D) CreateSprite(REF(Texture2D) texture);
        void DestroySprite(REF(EnhancedSprite2D) sprite);

        // Batch rendering
        void BeginFrame(const glm::mat4& viewProjectionMatrix);
        void EndFrame();

        void DrawSprite(REF(EnhancedSprite2D) sprite);

        // Layer-based rendering
        void DrawSprite(REF(EnhancedSprite2D) sprite, int layer);
        void SetLayerBlendMode(int layer, SpriteBlendMode blendMode);

        // Statistics
        SpriteBatch::Stats GetFrameStats() const;

    private:
        SpriteManager() = default;
        ~SpriteManager() = default;

        std::unique_ptr<SpriteBatch> m_SpriteBatch;
        std::unordered_map<int, std::vector<REF(EnhancedSprite2D)>> m_LayeredSprites;
        std::unordered_map<int, SpriteBlendMode> m_LayerBlendModes;

        // Resource tracking
        std::vector<std::weak_ptr<EnhancedSprite2D>> m_ManagedSprites;

        void CleanupDestroyedSprites();
    };

    // Utility functions for C3Engine compatibility
    namespace SpriteUtils {
        // Color conversion utilities
        glm::vec4 ARGBToVec4(uint32_t argb);
        uint32_t Vec4ToARGB(const glm::vec4& color);

        // Rectangle utilities
        glm::vec4 RectToVec4(int x, int y, int width, int height);
        glm::ivec4 Vec4ToRect(const glm::vec4& rect);

        // UV coordinate helpers
        glm::vec4 PixelRectToUV(const glm::ivec4& pixelRect, const glm::ivec2& textureSize);
        glm::ivec4 UVToPixelRect(const glm::vec4& uvRect, const glm::ivec2& textureSize);

        // Animation helpers
        std::vector<EnhancedSprite2D::AnimationFrame> CreateSpriteSheetAnimation(
            const glm::ivec2& frameSize, const glm::ivec2& sheetSize,
            float frameDuration, const std::string& baseName = "frame");
    }

} // namespace Cherry