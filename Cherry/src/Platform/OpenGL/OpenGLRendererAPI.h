#pragma once
#include "Cherry/Renderer/RendererAPI.h"

namespace Cherry {

    class OpenGLRendererAPI : public RendererAPI {
    public:
        virtual void Init() override;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;

        // Existing method
        virtual void DrawIndexed(const REF(VertexArray)& vertexArray) override;

        // NEW METHODS - Add these implementations
        virtual void DrawIndexed(const REF(VertexArray)& vertexArray, uint32_t indexCount) override;
        virtual void SetLineWidth(float width) override;
        virtual void DrawLines(const REF(VertexArray)& vertexArray, uint32_t vertexCount) override;
    };
}