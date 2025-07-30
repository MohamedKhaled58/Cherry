#pragma once

#include "Cherry/Renderer/RendererAPI.h"

// Cherry/src/Cherry/Renderer/RenderCommand.h
#pragma once
#include "Cherry/Renderer/RendererAPI.h"

namespace Cherry {

    class RenderCommand {
    public:
        inline static void Init() {
            s_RendererAPI->Init();
        }

        inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        inline static void SetClearColor(const glm::vec4& color) {
            s_RendererAPI->SetClearColor(color);
        }

        inline static void Clear() {
            s_RendererAPI->Clear();
        }

        inline static void DrawIndexed(const REF(VertexArray)& vertexArray) {
            s_RendererAPI->DrawIndexed(vertexArray);
        }

        // Add these new methods:
        inline static void DrawIndexed(const REF(VertexArray)& vertexArray, uint32_t indexCount) {
            s_RendererAPI->DrawIndexed(vertexArray, indexCount);
        }

        inline static void SetLineWidth(float width) {
            s_RendererAPI->SetLineWidth(width);
        }

        inline static void DrawLines(const REF(VertexArray)& vertexArray, uint32_t vertexCount) {
            s_RendererAPI->DrawLines(vertexArray, vertexCount);
        }

    private:
        static RendererAPI* s_RendererAPI;
    };
}