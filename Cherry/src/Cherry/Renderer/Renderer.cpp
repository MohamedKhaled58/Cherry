#include "CHpch.h"
#include "Renderer.h"
#include "Cherry/Renderer/Renderer2D.h"
#include "Cherry/Renderer/Shader.h"
#include <Platform/OpenGL/OpenGLShader.h>


namespace Cherry {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;  // Initialize to nullptr

	void Renderer::Init()
	{
		CH_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		CH_PROFILE_FUNCTION();

		Renderer2D::Shutdown();
		delete m_SceneData;
		m_SceneData = nullptr;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);

	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const REF(Shader)& shader, const REF(VertexArray)& vertexArray,const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
		//mi->Bind();

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::Flush() {
	}




}