#include "CHpch.h"
#include "Cherry/Renderer/Shader.h"

#include "Cherry/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Cherry {

	REF(Shader) Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    CH_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return SmartPointer::CreateRef<OpenGLShader>(filepath);
		}

		CH_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	REF(Shader) Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    CH_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return SmartPointer::CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		CH_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const REF(Shader)& shader)
	{
		CH_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const REF(Shader)& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	REF(Shader) ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	REF(Shader) ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	REF(Shader) ShaderLibrary::Get(const std::string& name)
	{
		CH_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}