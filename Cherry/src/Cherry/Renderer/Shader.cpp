#include "CHpch.h"
#include "Shader.h"
#include "Cherry/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Cherry {

	Shader* Shader::Create(const std::string& filepath)
	{

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); 
				return nullptr;
			case RendererAPI::API::OpenGL:	return new OpenGLShader(filepath);
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
		return nullptr;

	}

	Shader* Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::API::OpenGL:	return new OpenGLShader(vertexSource, fragmentSource);
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
		return nullptr;

	}

}
