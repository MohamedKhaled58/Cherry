// Cherry/src/Cherry/Renderer/Buffer.cpp
#include "CHpch.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Cherry {

    REF(VertexBuffer) VertexBuffer::Create(float* vertices, uint32_t size) {
        switch (Renderer::GetAPI()) {
        case RendererAPI::API::None:
            CH_CORE_ASSERT(false, "RendererAPI::None is not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        }

        CH_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    REF(VertexBuffer) VertexBuffer::Create(uint32_t size) {
        switch (Renderer::GetAPI()) {
        case RendererAPI::API::None:
            CH_CORE_ASSERT(false, "RendererAPI::None is not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(size);
        }

        CH_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    REF(IndexBuffer) IndexBuffer::Create(uint32_t* indices, uint32_t count) {
        switch (Renderer::GetAPI()) {
        case RendererAPI::API::None:
            CH_CORE_ASSERT(false, "RendererAPI::None is not supported!");
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLIndexBuffer>(indices, count);
        }

        CH_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}