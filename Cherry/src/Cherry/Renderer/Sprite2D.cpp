// Cherry/src/Cherry/Renderer/EnhancedSprite2D.cpp
#include "CHpch.h"
#include "Sprite2D.h"
#include "Cherry/Renderer/RenderCommand.h"
#include "Cherry/Core/Application.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Cherry {

    // EnhancedSprite2D Implementation
    EnhancedSprite2D::EnhancedSprite2D() {
        SetupDefaultVertices();
    }

    EnhancedSprite2D::EnhancedSprite2D(REF(Texture2D) texture) : m_Texture(texture) {
        SetupDefaultVertices();
        if (m_Texture) {
            m_Size = { static_cast<float>(m_Texture->GetWidth()), static_cast<float>(m_Texture->GetHeight()) };
            UpdateVertices();
        }
    }

    void EnhancedSprite2D::SetupDefaultVertices() {
        // Setup default quad vertices
        m_Vertices[0] = { {-0.5f, -0.5f, 0.0f}, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} };
        m_Vertices[1] = { { 0.5f, -0.5f, 0.0f}, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} };
        m_Vertices[2] = { { 0.5f,  0.5f, 0.0f}, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} };
        m_Vertices[3] = { {-0.5f,  0.5f, 0.0f}, 1.0f, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} };
    }

    void EnhancedSprite2D::SetVertexColor(uint32_t ltColor, uint32_t rtColor, uint32_t lbColor, uint32_t rbColor) {
        m_Vertices[0].Color = ColorToVec4(ltColor); // Top-left
        m_Vertices[1].Color = ColorToVec4(rtColor); // Top-right
        m_Vertices[2].Color = ColorToVec4(rbColor); // Bottom-right
        m_Vertices[3].Color = ColorToVec4(lbColor); // Bottom-left
    }

    void EnhancedSprite2D::SetColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
        glm::vec4 color = { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
        SetColor(color);
    }

    void EnhancedSprite2D::SetColor(const glm::vec4& color) {
        for (auto& vertex : m_Vertices) {
            vertex.Color = color;
        }
    }

    void EnhancedSprite2D::SetCoordinates(const glm::ivec4* srcRect, int x, int y, uint32_t width, uint32_t height) {
        if (srcRect) {
            SetCoordinates(srcRect->x, srcRect->y, srcRect->z, srcRect->w, x, y, width, height);
        }
        else {
            SetCoordinates(0, 0, m_Texture ? m_Texture->GetWidth() : 32,
                m_Texture ? m_Texture->GetHeight() : 32, x, y, width, height);
        }
    }

    void EnhancedSprite2D::SetCoordinates(int srcX, int srcY, int srcW, int srcH, int x, int y, uint32_t width, uint32_t height) {
        // Set destination position and size
        m_Position = { static_cast<float>(x), static_cast<float>(y) };
        m_Size = { width > 0 ? static_cast<float>(width) : static_cast<float>(srcW),
                  height > 0 ? static_cast<float>(height) : static_cast<float>(srcH) };

        // Set UV coordinates if texture exists
        if (m_Texture) {
            float invWidth = 1.0f / static_cast<float>(m_Texture->GetWidth());
            float invHeight = 1.0f / static_cast<float>(m_Texture->GetHeight());

            float u1 = static_cast<float>(srcX) * invWidth;
            float v1 = static_cast<float>(srcY) * invHeight;
            float u2 = static_cast<float>(srcX + srcW) * invWidth;
            float v2 = static_cast<float>(srcY + srcH) * invHeight;

            SetUVRect({ u1, v1, u2, v2 });
        }

        UpdateVertices();
    }

    void EnhancedSprite2D::SetUVRect(float u1, float v1, float u2, float v2) {
        SetUVRect({ u1, v1, u2, v2 });
    }

    void EnhancedSprite2D::SetUVRect(const glm::vec4& uvRect) {
        m_Vertices[0].UV = { uvRect.x, uvRect.y }; // Top-left
        m_Vertices[1].UV = { uvRect.z, uvRect.y }; // Top-right
        m_Vertices[2].UV = { uvRect.z, uvRect.w }; // Bottom-right
        m_Vertices[3].UV = { uvRect.x, uvRect.w }; // Bottom-left
    }

    glm::vec4 EnhancedSprite2D::GetUVRect() const {
        return { m_Vertices[0].UV.x, m_Vertices[0].UV.y, m_Vertices[2].UV.x, m_Vertices[2].UV.y };
    }

    void EnhancedSprite2D::SetPosition(const glm::vec2& position) {
        m_Position = position;
        UpdateVertices();
    }

    void EnhancedSprite2D::SetPosition(float x, float y) {
        SetPosition({ x, y });
    }

    glm::vec2 EnhancedSprite2D::GetPosition() const {
        return m_Position;
    }

    void EnhancedSprite2D::SetSize(const glm::vec2& size) {
        m_Size = size;
        UpdateVertices();
    }

    void EnhancedSprite2D::SetSize(float width, float height) {
        SetSize({ width, height });
    }

    glm::vec2 EnhancedSprite2D::GetSize() const {
        return m_Size;
    }

    void EnhancedSprite2D::SetRotation(float rotation) {
        m_Rotation = rotation;
        UpdateVertices();
    }

    void EnhancedSprite2D::SetScale(const glm::vec2& scale) {
        m_Scale = scale;
        UpdateVertices();
    }

    void EnhancedSprite2D::SetScale(float scale) {
        SetScale({ scale, scale });
    }

    void EnhancedSprite2D::SetOrigin(const glm::vec2& origin) {
        m_Origin = origin;
        UpdateVertices();
    }

    void EnhancedSprite2D::Mirror() {
        // Swap UV coordinates horizontally
        std::swap(m_Vertices[0].UV.x, m_Vertices[1].UV.x);
        std::swap(m_Vertices[2].UV.x, m_Vertices[3].UV.x);
    }

    void EnhancedSprite2D::FlipVertical() {
        // Swap UV coordinates vertically
        std::swap(m_Vertices[0].UV.y, m_Vertices[3].UV.y);
        std::swap(m_Vertices[1].UV.y, m_Vertices[2].UV.y);
    }

    void EnhancedSprite2D::FlipHorizontal() {
        Mirror();
    }

    bool EnhancedSprite2D::Draw(SpriteBlendMode blendMode) {
        if (!m_Texture) return false;

        // This would be implemented with immediate mode rendering
        // For batched rendering, use SubmitToBatch instead
        CH_CORE_WARN("EnhancedSprite2D::Draw() - Use SpriteBatch for better performance");
        return true;
    }

    void EnhancedSprite2D::UpdateVertices() {
        glm::mat4 transform = GetTransformMatrix();

        // Apply transform to vertex positions
        glm::vec4 positions[4] = {
            {-0.5f, -0.5f, 0.0f, 1.0f},  // Top-left
            { 0.5f, -0.5f, 0.0f, 1.0f},  // Top-right
            { 0.5f,  0.5f, 0.0f, 1.0f},  // Bottom-right
            {-0.5f,  0.5f, 0.0f, 1.0f}   // Bottom-left
        };

        for (size_t i = 0; i < 4; ++i) {
            glm::vec4 transformedPos = transform * positions[i];
            m_Vertices[i].Position = { transformedPos.x, transformedPos.y, transformedPos.z };
        }

        // Apply opacity to vertex colors
        for (auto& vertex : m_Vertices) {
            vertex.Color.a *= m_Opacity;
        }
    }

    glm::mat4 EnhancedSprite2D::GetTransformMatrix() const {
        glm::mat4 transform = glm::mat4(1.0f);

        // Apply transformations in order: Scale -> Rotate -> Translate
        transform = glm::translate(transform, { m_Position.x, m_Position.y, 0.0f });

        if (m_Rotation != 0.0f) {
            transform = glm::rotate(transform, m_Rotation, { 0.0f, 0.0f, 1.0f });
        }

        // Apply origin offset
        glm::vec2 originOffset = {
            -m_Origin.x * m_Size.x,
            -m_Origin.y * m_Size.y
        };
        transform = glm::translate(transform, { originOffset.x, originOffset.y, 0.0f });

        // Apply scale and size
        glm::vec3 finalScale = { m_Size.x * m_Scale.x, m_Size.y * m_Scale.y, 1.0f };
        transform = glm::scale(transform, finalScale);

        return transform;
    }

    glm::vec4 EnhancedSprite2D::ColorToVec4(uint32_t color) const {
        float a = ((color >> 24) & 0xFF) / 255.0f;
        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8) & 0xFF) / 255.0f;
        float b = (color & 0xFF) / 255.0f;
        return { r, g, b, a };
    }

    uint32_t EnhancedSprite2D::Vec4ToColor(const glm::vec4& color) const {
        uint8_t r = static_cast<uint8_t>(glm::clamp(color.r * 255.0f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(glm::clamp(color.g * 255.0f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(glm::clamp(color.b * 255.0f, 0.0f, 255.0f));
        uint8_t a = static_cast<uint8_t>(glm::clamp(color.a * 255.0f, 0.0f, 255.0f));
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    // SpriteBatch Implementation
    SpriteBatch::SpriteBatch(size_t maxSprites) : m_MaxSprites(maxSprites) {
        Initialize();
    }

    SpriteBatch::~SpriteBatch() {
        Shutdown();
    }

    void SpriteBatch::Initialize() {
        // Allocate memory for vertices and indices
        m_Vertices.reserve(m_MaxSprites * 4);
        m_Indices.reserve(m_MaxSprites * 6);

        // Generate indices for quads
        for (size_t i = 0; i < m_MaxSprites; ++i) {
            uint32_t offset = static_cast<uint32_t>(i * 4);
            m_Indices.push_back(offset + 0);
            m_Indices.push_back(offset + 1);
            m_Indices.push_back(offset + 2);
            m_Indices.push_back(offset + 2);
            m_Indices.push_back(offset + 3);
            m_Indices.push_back(offset + 0);
        }

        // Create OpenGL objects
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_MaxSprites * 4 * sizeof(BatchVertex), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), m_Indices.data(), GL_STATIC_DRAW);

        // Set vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)offsetof(BatchVertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)offsetof(BatchVertex, RHW));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)offsetof(BatchVertex, Color));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)offsetof(BatchVertex, UV));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(BatchVertex), (void*)offsetof(BatchVertex, TextureIndex));

        glBindVertexArray(0);

        // Load sprite batch shader
        m_Shader = Shader::Create("assets/shaders/SpriteBatch.glsl");
        if (!m_Shader) {
            CH_CORE_ERROR("Failed to load SpriteBatch shader");
        }
    }

    void SpriteBatch::Shutdown() {
        if (m_VAO) {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
        if (m_VBO) {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        if (m_EBO) {
            glDeleteBuffers(1, &m_EBO);
            m_EBO = 0;
        }
    }

    void SpriteBatch::Begin(const glm::mat4& viewProjectionMatrix) {
        CH_CORE_ASSERT(!m_HasBegun, "SpriteBatch::Begin called twice without End");

        m_HasBegun = true;
        m_ViewProjectionMatrix = viewProjectionMatrix;
        m_CurrentSprite = 0;
        m_CurrentTexture = 0;
        m_Vertices.clear();

        // Reset texture slots
        for (size_t i = 0; i < m_Textures.size(); ++i) {
            m_Textures[i] = nullptr;
        }

        m_Stats = {}; // Reset stats
    }

    void SpriteBatch::End() {
        CH_CORE_ASSERT(m_HasBegun, "SpriteBatch::End called without Begin");

        Flush();
        m_HasBegun = false;
    }

    void SpriteBatch::Draw(REF(EnhancedSprite2D) sprite) {
        if (!sprite || !sprite->GetTexture()) return;

        float textureIndex = GetTextureIndex(sprite->GetTexture());

        // Add vertices
        const auto& vertices = sprite->GetVertices();
        for (const auto& vertex : vertices) {
            BatchVertex batchVertex = vertex;
            batchVertex.TextureIndex = textureIndex;
            m_Vertices.push_back(batchVertex);
        }

        m_CurrentSprite++;
        m_Stats.SpritesDrawn++;
        m_Stats.VerticesSubmitted += 4;

        // Flush if we've reached capacity
        if (m_CurrentSprite >= m_MaxSprites) {
            Flush();
        }
    }

    void SpriteBatch::Flush() {
        if (m_Vertices.empty() || !m_Shader) return;

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_Vertices.size() * sizeof(BatchVertex), m_Vertices.data());

        // Bind textures
        for (size_t i = 0; i < m_CurrentTexture; ++i) {
            if (m_Textures[i]) {
                m_Textures[i]->Bind(static_cast<uint32_t>(i));
            }
        }

        // Set shader uniforms
        m_Shader->Bind();
        m_Shader->SetMat4("u_ViewProjection", m_ViewProjectionMatrix);

        // Set texture samplers
        int samplers[32];
        for (int i = 0; i < 32; ++i) {
            samplers[i] = i;
        }
        m_Shader->SetIntArray("u_Textures", samplers, 32);

        // Draw
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_CurrentSprite * 6), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        m_Stats.DrawCalls++;
        m_Stats.TextureBinds += m_CurrentTexture;

        // Reset for next batch
        m_Vertices.clear();
        m_CurrentSprite = 0;
        m_CurrentTexture = 0;
    }

    float SpriteBatch::GetTextureIndex(REF(Texture2D) texture) {
        // Check if texture is already in the batch
        for (size_t i = 0; i < m_CurrentTexture; ++i) {
            if (m_Textures[i] && *m_Textures[i] == *texture) {
                return static_cast<float>(i);
            }
        }

        // Add new texture if there's space
        if (m_CurrentTexture < m_Textures.size()) {
            m_Textures[m_CurrentTexture] = texture;
            return static_cast<float>(m_CurrentTexture++);
        }

        // No space, flush and add texture
        Flush();
        m_Textures[0] = texture;
        m_CurrentTexture = 1;
        return 0.0f;
    }

    // SpriteManager Implementation
    void SpriteManager::Initialize() {
        m_SpriteBatch = std::make_unique<SpriteBatch>(10000);
        CH_CORE_INFO("SpriteManager initialized");
    }

    void SpriteManager::Shutdown() {
        m_SpriteBatch.reset();
        m_LayeredSprites.clear();
        m_LayerBlendModes.clear();
        CleanupDestroyedSprites();
        CH_CORE_INFO("SpriteManager shut down");
    }

    REF(EnhancedSprite2D) SpriteManager::CreateSprite(const std::string& texturePath) {
        auto texture = Texture2D::Create(texturePath);
        if (!texture) {
            CH_CORE_ERROR("Failed to load texture: {}", texturePath);
            return nullptr;
        }
        return CreateSprite(texture);
    }

    REF(EnhancedSprite2D) SpriteManager::CreateSprite(REF(Texture2D) texture) {
        auto sprite = std::make_shared<EnhancedSprite2D>(texture);
        m_ManagedSprites.push_back(sprite);
        return sprite;
    }

    void SpriteManager::BeginFrame(const glm::mat4& viewProjectionMatrix) {
        if (m_SpriteBatch) {
            m_SpriteBatch->Begin(viewProjectionMatrix);
        }
        CleanupDestroyedSprites();
    }

    void SpriteManager::EndFrame() {
        // Draw layered sprites
        for (auto& [layer, sprites] : m_LayeredSprites) {
            // Set blend mode for layer
            auto blendModeIt = m_LayerBlendModes.find(layer);
            if (blendModeIt != m_LayerBlendModes.end()) {
                // TODO: Set OpenGL blend mode based on blendModeIt->second
            }

            for (auto& sprite : sprites) {
                if (sprite) {
                    m_SpriteBatch->Draw(sprite);
                }
            }
            sprites.clear();
        }

        if (m_SpriteBatch) {
            m_SpriteBatch->End();
        }
    }

    void SpriteManager::DrawSprite(REF(EnhancedSprite2D) sprite) {
        DrawSprite(sprite, 0); // Default layer
    }

    void SpriteManager::DrawSprite(REF(EnhancedSprite2D) sprite, int layer) {
        m_LayeredSprites[layer].push_back(sprite);
    }

    void SpriteManager::CleanupDestroyedSprites() {
        m_ManagedSprites.erase(
            std::remove_if(m_ManagedSprites.begin(), m_ManagedSprites.end(),
                [](const std::weak_ptr<EnhancedSprite2D>& weak) {
                    return weak.expired();
                }),
            m_ManagedSprites.end()
        );
    }

    SpriteBatch::Stats SpriteManager::GetFrameStats() const {
        return m_SpriteBatch ? m_SpriteBatch->GetStats() : SpriteBatch::Stats{};
    }

    // Utility Functions
    namespace SpriteUtils {
        glm::vec4 ARGBToVec4(uint32_t argb) {
            float a = ((argb >> 24) & 0xFF) / 255.0f;
            float r = ((argb >> 16) & 0xFF) / 255.0f;
            float g = ((argb >> 8) & 0xFF) / 255.0f;
            float b = (argb & 0xFF) / 255.0f;
            return { r, g, b, a };
        }

        uint32_t Vec4ToARGB(const glm::vec4& color) {
            uint8_t a = static_cast<uint8_t>(glm::clamp(color.a * 255.0f, 0.0f, 255.0f));
            uint8_t r = static_cast<uint8_t>(glm::clamp(color.r * 255.0f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(glm::clamp(color.g * 255.0f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(glm::clamp(color.b * 255.0f, 0.0f, 255.0f));
            return (a << 24) | (r << 16) | (g << 8) | b;
        }

        glm::vec4 RectToVec4(int x, int y, int width, int height) {
            return { static_cast<float>(x), static_cast<float>(y),
                    static_cast<float>(width), static_cast<float>(height) };
        }

        glm::ivec4 Vec4ToRect(const glm::vec4& rect) {
            return { static_cast<int>(rect.x), static_cast<int>(rect.y),
                    static_cast<int>(rect.z), static_cast<int>(rect.w) };
        }
    }

} // namespace Cherry