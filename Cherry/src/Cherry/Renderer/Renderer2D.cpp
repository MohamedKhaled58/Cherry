#include "CHpch.h"
#include "Renderer2D.h"

#include "Cherry/Renderer/VertexArray.h"
#include "Cherry/Renderer/Shader.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/RenderCommand.h"

#include "Cherry/Core/Core.h"
#include <glm/ext/matrix_transform.hpp>

namespace Cherry {
    
    struct Renderer2DStorage
    {
        REF(VertexArray) QuadVertexArray;
        REF(Shader) FlatColorShader;
        REF(Shader) TextureShader;
    };

    static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
        s_Data = new Renderer2DStorage();

        float SquareVertices[5 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f,   0.0f,
             0.5f, -0.5f, 0.0f, 1.0f,   0.0f,
             0.5f,  0.5f, 0.0f, 1.0f,   1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f,   1.0f
        };

        uint32_t SquareIndices[6] = { 0, 1, 2, 2, 3, 0 };

        s_Data->QuadVertexArray = VertexArray::Create();

        // Creating Vertex Buffer
        REF(VertexBuffer)SquareVB;

        SquareVB = (VertexBuffer::Create(SquareVertices, sizeof(SquareVertices)));
        BufferLayout Squarelayout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
        };

        SquareVB->SetLayout(Squarelayout);
        s_Data->QuadVertexArray->AddVertexBuffer(SquareVB);

        // Creating Index Buffer
        REF(IndexBuffer)SquareIB;

        SquareIB = IndexBuffer::Create(SquareIndices, sizeof(SquareIndices) / sizeof(uint32_t));
        s_Data->QuadVertexArray->SetIndexBuffer(SquareIB);

        s_Data->FlatColorShader = Shader::Create("assets/shaders/FlatColor.glsl");
        s_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetInt("u_Texture",0);

	}
	void Renderer2D::Shutdown()
	{
        delete s_Data;
	}
	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
       s_Data->FlatColorShader->Bind();
       s_Data->FlatColorShader->SetMat4("u_ViewProjection",camera.GetViewProjectionMatrix());

       s_Data->TextureShader->Bind();
       s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

	}
	void Renderer2D::EndScene()
	{
        //Flush();
	}

    void Renderer2D::DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color)
    {
        s_Data->FlatColorShader->Bind();

        // Build transform: Translate → Rotate (Z-axis) → Scale
        glm::mat4 transform = 
              glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f))
            * glm::scale(glm::mat4(1.0f), { size.x,size.y,1.0f });

        s_Data->FlatColorShader->SetFloat4("u_Color", color);
        s_Data->FlatColorShader->SetMat4("u_Transform", transform);

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }


    void Renderer2D::DrawQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x,position.y,0.0f }, rotation, size, color);
    }




    void Renderer2D::DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const REF(Texture2D)& texture)
    {
        s_Data->TextureShader->Bind();
        texture->Bind();

        // Build transform: Translate → Rotate (Z-axis) → Scale
        glm::mat4 transform =
	        glm::translate(glm::mat4(1.0f), position)
	        * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f))
	        * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        s_Data->TextureShader->SetMat4("u_Transform", transform);

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }


    void Renderer2D::DrawQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const REF(Texture2D)& texture)
    {
        DrawQuad({ position.x,position.y,0.0f }, rotation, size, texture);
    }
}