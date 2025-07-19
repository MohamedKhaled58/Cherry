#include "CHpch.h"
#include "Renderer.h"
#include "Cherry/Renderer/Shader.h"


namespace Cherry {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;  // Initialize to nullptr

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray,const glm::mat4& transform)
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		//	MODEL MATRIX
		shader->UploadUniformMat4("u_Transform", transform);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}


}