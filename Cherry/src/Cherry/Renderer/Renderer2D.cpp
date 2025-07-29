#include "CHpch.h"
#include "Renderer2D.h"

#include "Cherry/Renderer/VertexArray.h"
#include "Cherry/Renderer/Shader.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/RenderCommand.h"

#include "Cherry/Core/Core.h"
#include <Platform/OpenGL/OpenGLShader.h>

namespace Cherry {
    
    struct Renderer2DStorage
    {
        REF(VertexArray) QuadVertexArray;
        REF(Shader) FlatColorShader;
    };

    static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
        s_Data = new Renderer2DStorage();

        float SquareVertices[5 * 4] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f 
        };

        uint32_t SquareIndices[6] = { 0, 1, 2, 2, 3, 0 };

        s_Data->QuadVertexArray = VertexArray::Create();

        // Creating Vertex Buffer
        REF(VertexBuffer)SquareVB;

        SquareVB = (VertexBuffer::Create(SquareVertices, sizeof(SquareVertices)));
        BufferLayout Squarelayout = {
            { ShaderDataType::Float3, "a_Position" }
        };
        SquareVB->SetLayout(Squarelayout);
        s_Data->QuadVertexArray->AddVertexBuffer(SquareVB);

        // Creating Index Buffer
        REF(IndexBuffer)SquareIB;

        SquareIB = IndexBuffer::Create(SquareIndices, sizeof(SquareIndices) / sizeof(uint32_t));
        s_Data->QuadVertexArray->SetIndexBuffer(SquareIB);
        s_Data->FlatColorShader = Shader::Create("assets/shaders/FlatColor.glsl");
	}
	void Renderer2D::Shutdown()
	{
        delete s_Data;
	}
	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
       std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
       std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_ViewProjection",camera.GetViewProjectionMatrix());
       std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_Transform",glm::mat4(1.0f));

	}
	void Renderer2D::EndScene()
	{
        //Flush();
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformFloat4("u_Color", color);
        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x,position.y,0.0f }, size, color);
    }
}