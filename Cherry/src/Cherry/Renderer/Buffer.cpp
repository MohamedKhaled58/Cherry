#include "CHpch.h"
#include "Buffer.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"


namespace Cherry {

	REF(VertexBuffer) VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::API::OpenGL:	return std::make_shared<OpenGLVertexBuffer>(vertices, size);
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::API::OpenGL:	return new OpenGLIndexBuffer(indices, count);
		}

		return nullptr;
	}
}