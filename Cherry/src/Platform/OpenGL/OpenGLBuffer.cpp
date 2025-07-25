#include "CHpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Cherry {
	//////////////////////////////////////////
	//	VERTEX BUFFER	//////////////////////
	//////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		//TODO: FIX Hard Coded STATIC DRAW
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
		m_RendererID = 0;
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	//////////////////////////////////////////
	//	INDEX BUFFER	//////////////////////
	//////////////////////////////////////////
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		:m_Count(count)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		//TODO: FIX Hard Coded STATIC DRAW
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
		m_RendererID = 0;
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}