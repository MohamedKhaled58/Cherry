#include "CHpch.h"
#include "Texture.h"

#include "Cherry/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Cherry {

	REF(Texture2D) Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    CH_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLTexture2D>(path);
		}

		CH_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}