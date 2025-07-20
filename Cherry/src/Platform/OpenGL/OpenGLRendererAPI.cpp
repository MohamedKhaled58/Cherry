#include "CHpch.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace Cherry {

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	}
	void OpenGLRendererAPI::DrawIndexed(const REF(VertexArray)& vertexArray)
	{
		//TODO
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffers()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}