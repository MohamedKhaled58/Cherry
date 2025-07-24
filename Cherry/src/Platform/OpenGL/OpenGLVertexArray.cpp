#include "Chpch.h"
#include <glad/glad.h>
#include "OpenGLVertexArray.h"

namespace Cherry {

	static GLenum ShaderDataTypeToOpenGlBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Cherry::ShaderDataType::Float:		return GL_FLOAT;
			case Cherry::ShaderDataType::Float2:		return GL_FLOAT;
			case Cherry::ShaderDataType::Float3:		return GL_FLOAT;
			case Cherry::ShaderDataType::Float4:		return GL_FLOAT;
			case Cherry::ShaderDataType::Mat3:			return GL_FLOAT;
			case Cherry::ShaderDataType::Mat4:			return GL_FLOAT;
			case Cherry::ShaderDataType::Int:			return GL_INT;
			case Cherry::ShaderDataType::Int2:			return GL_INT;
			case Cherry::ShaderDataType::Int3:			return GL_INT;
			case Cherry::ShaderDataType::Int4:			return GL_INT;
			case Cherry::ShaderDataType::Bool:			return GL_BOOL;
		}
		CH_CORE_ASSERT(false, "UnKnown ShaderDataType");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);

	}
	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}
	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}
	void OpenGLVertexArray::AddVertexBuffer(const REF(VertexBuffer)& vertexBuffer)
	{
		//Check Why Not Working
		CH_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(),"Vertex Buffer Has No Layout!");
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(m_VertexBufferIndex);
			glVertexAttribPointer(
				m_VertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGlBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)(intptr_t)element.Offset);

			m_VertexBufferIndex++;
		}

		m_VertexBuffers.push_back(vertexBuffer);

	}
	void OpenGLVertexArray::SetIndexBuffer(const REF(IndexBuffer)& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();


		m_IndexBuffer = indexBuffer;
	}
}