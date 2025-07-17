#include "CHpch.h"
#include "Buffer.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"


namespace Cherry {

	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size, uint32_t usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:		CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::OpenGL:	return new OpenGLVertexBuffer(vertices, size, usage);
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count, uint32_t usage)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:		CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::OpenGL:	return new OpenGLIndexBuffer(indices, count, usage);
		}

		return nullptr;
	}
}