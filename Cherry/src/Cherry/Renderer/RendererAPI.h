#pragma once
#include <glm/glm.hpp>
#include "Cherry/Renderer/VertexArray.h"

namespace Cherry {

    class RendererAPI {
    public:
        enum class API {
            None = 0, OpenGL = 1
        };

    public:
        virtual ~RendererAPI() = default;

        virtual void Init() = 0;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        // Existing method
        virtual void DrawIndexed(const REF(VertexArray)& vertexArray) = 0;

        // NEW METHODS - Add these to fix compilation errors
        virtual void DrawIndexed(const REF(VertexArray)& vertexArray, uint32_t indexCount) = 0;
        virtual void SetLineWidth(float width) = 0;
        virtual void DrawLines(const REF(VertexArray)& vertexArray, uint32_t vertexCount) = 0;

        inline static API GetAPI() { return s_API; }

    private:
        static API s_API;
    };
}