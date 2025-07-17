#include "Chpch.h"
#include "VertexArray.h"
#include "Cherry/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"


namespace Cherry {
	VertexArray* VertexArray::Create()
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::API::OpenGL:	return new OpenGLVertexArray();
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
			return nullptr;

	}
}
