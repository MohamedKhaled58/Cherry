#include "CHpch.h"
#include "BatchRenderer2D.h"
#include "Cherry/Renderer/RenderCommand.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Cherry/Core/Debug/Profiler.h"

namespace Cherry {

    BatchRenderer2D::RendererData BatchRenderer2D::s_Data;

    void BatchRenderer2D::Init() {
        CH_PROFILE_FUNCTION();

        // Initialize quad vertex positions
        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        // Create quad vertex array
        s_Data.QuadVertexArray = VertexArray::Create();

        s_Data.QuadVertexBuffer = VertexBuffer::Create(RenderLimits::MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float,  "a_TilingFactor" },
            { ShaderDataType::Float2, "a_Normal" },
            { ShaderDataType::Float,  "a_Roughness" },
            { ShaderDataType::Float,  "a_Metallic" }
            });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[RenderLimits::MaxVertices];

        // Create index buffer
        uint32_t* quadIndices = new uint32_t[RenderLimits::MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < RenderLimits::MaxIndices; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        REF(IndexBuffer) quadIB = IndexBuffer::Create(quadIndices, RenderLimits::MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        // Create line vertex array
        s_Data.LineVertexArray = VertexArray::Create();
        s_Data.LineVertexBuffer = VertexBuffer::Create(RenderLimits::MaxVertices * sizeof(QuadVertex));
        s_Data.LineVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" }
            });
        s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase = new QuadVertex[RenderLimits::MaxVertices];

        // Create circle vertex array
        s_Data.CircleVertexArray = VertexArray::Create();
        s_Data.CircleVertexBuffer = VertexBuffer::Create(RenderLimits::MaxVertices * sizeof(QuadVertex));
        s_Data.CircleVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_LocalPosition" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float,  "a_Thickness" },
            { ShaderDataType::Float,  "a_Fade" }
            });
        s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
        s_Data.CircleVertexArray->SetIndexBuffer(quadIB); // Reuse quad indices
        s_Data.CircleVertexBufferBase = new QuadVertex[RenderLimits::MaxVertices];

        // Create white texture
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        // Initialize texture slots
        int32_t samplers[RenderLimits::MaxTextureSlots];
        for (uint32_t i = 0; i < RenderLimits::MaxTextureSlots; i++)
            samplers[i] = i;

        // Load shaders
        s_Data.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
        s_Data.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
        s_Data.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
        s_Data.TextShader = Shader::Create("assets/shaders/Renderer2D_Text.glsl");

        // Set texture slots
        s_Data.QuadShader->Bind();
        s_Data.QuadShader->SetIntArray("u_Textures", samplers, RenderLimits::MaxTextureSlots);

        // Set first texture slot to white texture
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        CH_CORE_INFO("BatchRenderer2D initialized with {} max quads", RenderLimits::MaxQuads);
    }

    void BatchRenderer2D::Shutdown() {
        CH_PROFILE_FUNCTION();

        delete[] s_Data.QuadVertexBufferBase;
        delete[] s_Data.LineVertexBufferBase;
        delete[] s_Data.CircleVertexBufferBase;
    }

    void BatchRenderer2D::BeginScene(const OrthographicCamera& camera) {
        CH_PROFILE_FUNCTION();

        s_Data.ViewProjectionMatrix = camera.GetViewProjectionMatrix();

        s_Data.QuadShader->Bind();
        s_Data.QuadShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

        s_Data.LineShader->Bind();
        s_Data.LineShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

        s_Data.CircleShader->Bind();
        s_Data.CircleShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

        StartBatch();
    }

    void BatchRenderer2D::BeginScene(const glm::mat4& viewProjectionMatrix) {
        CH_PROFILE_FUNCTION();

        s_Data.ViewProjectionMatrix = viewProjectionMatrix;

        s_Data.QuadShader->Bind();
        s_Data.QuadShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

        s_Data.LineShader->Bind();
        s_Data.LineShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

        s_Data.CircleShader->Bind();
        s_Data.CircleShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

        StartBatch();
    }

    void BatchRenderer2D::EndScene() {
        CH_PROFILE_FUNCTION();

        Flush();
    }

    void BatchRenderer2D::StartBatch() {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void BatchRenderer2D::Flush() {
        CH_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

            // Bind textures
            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->Bind(i);

            s_Data.QuadShader->Bind();
            RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.LineVertexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
            s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

            s_Data.LineShader->Bind();
            RenderCommand::SetLineWidth(s_Data.LineWidth);
            RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.CircleIndexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
            s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);

            s_Data.CircleShader->Bind();
            RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
            s_Data.Stats.DrawCalls++;
        }
    }

    void BatchRenderer2D::NextBatch() {
        Flush();
        StartBatch();
    }

    void BatchRenderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID) {
        CH_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
        const float tilingFactor = 1.0f;

        if (s_Data.QuadIndexCount >= RenderLimits::MaxIndices)
            NextBatch();

        for (size_t i = 0; i < quadVertexCount; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void BatchRenderer2D::DrawQuad(const glm::mat4& transform, REF(Texture2D) texture,
        float tilingFactor, const glm::vec4& tintColor, int entityID) {
        CH_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

        if (s_Data.QuadIndexCount >= RenderLimits::MaxIndices)
            NextBatch();

        float textureIndex = GetTextureIndex(texture);

        for (size_t i = 0; i < quadVertexCount; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    float BatchRenderer2D::GetTextureIndex(REF(Texture2D) texture) {
        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
            if (*s_Data.TextureSlots[i] == *texture) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            if (s_Data.TextureSlotIndex >= RenderLimits::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        return textureIndex;
    }

    void BatchRenderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
        s_Data.LineVertexBufferPtr->Position = p0;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexBufferPtr->Position = p1;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexCount += 2;
    }

    void BatchRenderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
        glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
        glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

        DrawLine(p0, p1, color);
        DrawLine(p1, p2, color);
        DrawLine(p2, p3, color);
        DrawLine(p3, p0, color);
    }

    void BatchRenderer2D::SetLineWidth(float width) {
        s_Data.LineWidth = width;
    }

    void BatchRenderer2D::EnableWireframe(bool enable) {
        s_Data.WireframeMode = enable;
    }

} // namespace Cherry