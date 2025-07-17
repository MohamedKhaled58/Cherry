#include "CHpch.h"
#include "Renderer.h"

namespace Cherry{

	void Renderer::BeginScene()
	{

	}
	void Renderer::EndScene()
	{

	}
	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
	{
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}