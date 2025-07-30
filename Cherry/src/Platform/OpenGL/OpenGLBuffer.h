// Cherry/src/Platform/OpenGL/OpenGLBuffer.h
#pragma once
#include "Cherry/Renderer/Buffer.h"

namespace Cherry {

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(uint32_t size);                    // New: dynamic buffer
        OpenGLVertexBuffer(float* vertices, uint32_t size);   // Existing: static buffer
        virtual ~OpenGLVertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetData(const void* data, uint32_t size) override;  // New method
        virtual const BufferLayout& GetLayout() const override { return m_Layout; }
        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

    private:
        uint32_t m_RendererID;
        uint32_t m_Size;        // Store buffer size
        BufferLayout m_Layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual uint32_t GetCount() const override { return m_Count; }

    private:
        uint32_t m_RendererID;
        uint32_t m_Count;
    };
}