#include "CHpch.h"
#include "Renderer.h"
#include "Cherry/Renderer/Shader.h"
#include <Platform/OpenGL/OpenGLShader.h>


namespace Cherry {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;  // Initialize to nullptr

	void Renderer::Init()
	{
		RenderCommand::Init();
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


}