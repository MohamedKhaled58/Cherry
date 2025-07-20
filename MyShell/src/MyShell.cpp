#include "Cherry.h"
#include <Platform/OpenGL/OpenGLShader.h>

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class ShellLayer : public Cherry::Layer
{
public:

    ShellLayer()
        :Layer("Shell"),m_Camera(-1.6f, 1.6f, -0.9f, 0.9f, -1.0f, 1.0f), m_CameraPosition({0.0f,0.0f,0.0f}), m_CameraRotation(0.0f)
    {
        ///////////////////////////
       // Triangle Setup
       ///////////////////////////
        float Vertices[3 * 7] =
        {
            //     Position            |      Color (RGBA)
            -0.5f, -0.5f,  0.0f,        1.0f, 0.5f, 0.0f, 1.0f,  // Left - Orange
             0.5f, -0.5f,  0.0f,        1.0f, 1.0f, 0.0f, 1.0f,  // Right - Yellow
             0.0f,  0.5f,  0.0f,        1.0f, 0.0f, 0.0f, 1.0f   // Top - Red
        };
        uint32_t Indices[3] = { 0 , 1 , 2 };

        /////////Setting Up Triangle//////////
        m_VertexArray.reset(Cherry::VertexArray::Create());

        // Creating Vertex Buffer
        REF(Cherry::VertexBuffer)m_VertexBuffer;
        m_VertexBuffer.reset(Cherry::VertexBuffer::Create(Vertices, sizeof(Vertices)));

        // Setting Buffer Layout
        Cherry::BufferLayout layout = {
            { Cherry::ShaderDataType::Float3, "a_Position" },
            { Cherry::ShaderDataType::Float4, "a_Color" }
        };
        m_VertexBuffer->SetLayout(layout);

        // Adding Vertex Buffer TO Vertex Array
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);

        // Creating Index Buffer
        REF(Cherry::IndexBuffer)m_IndexBuffer;
        m_IndexBuffer.reset(Cherry::IndexBuffer::Create(Indices, sizeof(Indices) / sizeof(uint32_t)));

        // Adding Index Buffer TO Vertex Array
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);

        ///////////////////////////
        // Square Setup - FIXED
        ///////////////////////////
        float SquareVertices[3 * 4] = {
            // x      y      z
            -0.5f, -0.5f, 0.0f,  // Bottom-left  (0)
             0.5f, -0.5f, 0.0f,  // Bottom-right (1)
             0.5f,  0.5f, 0.0f,  // Top-right    (2)
            -0.5f,  0.5f, 0.0f   // Top-left     (3)
        };
        uint32_t SquareIndices[6] = {
            0, 1, 2,  // Triangle 1
            2, 3, 0   // Triangle 2
        };

        /////////Setting Up Square//////////
        m_FlatColorVertexArray.reset(Cherry::VertexArray::Create());

        // Creating Vertex Buffer
        REF(Cherry::VertexBuffer)m_SquareVB;
        m_SquareVB.reset((Cherry::VertexBuffer::Create(SquareVertices, sizeof(SquareVertices))));
        Cherry::BufferLayout Squarelayout = {
            { Cherry::ShaderDataType::Float3, "a_Position" }
        };
        m_SquareVB->SetLayout(Squarelayout);
        m_FlatColorVertexArray->AddVertexBuffer(m_SquareVB);

        // Creating Index Buffer - FIXED: Actually add it to vertex array
        REF(Cherry::IndexBuffer)m_SquareIB;
        m_SquareIB.reset(Cherry::IndexBuffer::Create(SquareIndices, sizeof(SquareIndices) / sizeof(uint32_t)));
        m_FlatColorVertexArray->SetIndexBuffer(m_SquareIB);


        ///////////////////////////
        // Triangle Shader
        ///////////////////////////
        std::string vertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;

        out vec3 v_Position;
        out vec4 v_Color;

        void main()
        {
            v_Position = a_Position;
            v_Color = a_Color;
            gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
        }
    )";

        std::string fregmantSource = R"(
        #version 330 core
        layout(location = 0) out vec4 color;

        in vec3 v_Position;
        in vec4 v_Color;

        void main()
        {
            color = v_Color;
        }
    )";
        m_Shader.reset(Cherry::Shader::Create(vertexSource, fregmantSource));

        ///////////////////////////
        // Square Shader
        ///////////////////////////
        std::string flatColorShaderVertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;

        out vec3 v_Position;
        uniform mat4 u_ViewProjection;
        uniform mat4 u_Transform;
        void main()
        {
            v_Position = a_Position;
            gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
        }
    )";

        std::string flatColorShaderFragmentSrc = R"(
        #version 330 core
        layout(location = 0) out vec4 color;

        in vec3 v_Position;
        uniform vec3 u_Color;
        void main()
        {
            color = vec4(u_Color,1.0);
        }
    )";
        m_FlatColorShader.reset(Cherry::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));
    }

    virtual void OnImGuiRender() override
    {
        ImGui::Begin("Setting");
        ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }


    void OnUpdate(Cherry::TimeStep timeStep) override
    {
        CH_CLIENT_TRACE("Delta time {0}s  {1}ms ", timeStep.GetSeconds(),timeStep.GetMilliSeconds());

        if (Cherry::Input::IsKeyPressed(CH_KEY_LEFT))
            m_CameraPosition.x -= m_CameraMoveSpeed * timeStep;
        else if (Cherry::Input::IsKeyPressed(CH_KEY_RIGHT))
            m_CameraPosition.x += m_CameraMoveSpeed * timeStep;

        if (Cherry::Input::IsKeyPressed(CH_KEY_DOWN))
            m_CameraPosition.y -= m_CameraMoveSpeed * timeStep;
        else if (Cherry::Input::IsKeyPressed(CH_KEY_UP))
            m_CameraPosition.y += m_CameraMoveSpeed * timeStep;


        if (Cherry::Input::IsKeyPressed(CH_KEY_A))
            m_CameraRotation += m_CameraRotationSpeed * timeStep;
        else if (Cherry::Input::IsKeyPressed(CH_KEY_D))
            m_CameraRotation -= m_CameraRotationSpeed * timeStep;


        // Clear screen
        Cherry::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Cherry::RenderCommand::Clear();

        m_Camera.SetPosition(m_CameraPosition);
        m_Camera.SetRotation(m_CameraRotation);
        Cherry::Renderer::BeginScene(m_Camera);

        glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
        glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

       /* Cherry::MaterialRef material = new Cherry::Material(m_FlatColorShader);
        Cherry::MaterialInstanceRef mi = new Cherry::MaterialInstance(material);

        mi->SetValue("u_Color", redColor);
        mi->SetTexture("u_Texture", texture);
        squareMesh->SetMaterial(mi);*/

        std::dynamic_pointer_cast<Cherry::OpenGLShader>(m_FlatColorShader)->Bind();
        std::dynamic_pointer_cast<Cherry::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color",m_SquareColor);


        static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        for (int y = 0; y < 20; y++)
        {
            for (int x = 0; x < 20; x++)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;

                Cherry::Renderer::Submit(m_FlatColorShader, m_FlatColorVertexArray, transform);

            }
        }
       

        Cherry::Renderer::Submit(m_Shader, m_VertexArray);

        Cherry::Renderer::EndScene();
    }

    void OnEvent(Cherry::Event& event) override
    {
    }

    private:
        REF(Cherry::Shader) m_Shader;
        REF(Cherry::VertexArray) m_VertexArray;

        REF(Cherry::Shader) m_FlatColorShader;
        REF(Cherry::VertexArray) m_FlatColorVertexArray;
        Cherry::OrthographicCamera m_Camera;

        glm::vec3 m_CameraPosition;
        float m_CameraRotation;

        float m_CameraMoveSpeed = 1.0f;
        float m_CameraRotationSpeed = 90.0f;

        glm::vec3 m_SquareColor = { 0.3f,0.1f,0.8f };
};


class MyShell : public Cherry::Application
{
public:
	MyShell()
	{
		PushLayer(new ShellLayer);
		CH_CLIENT_INFO("Pushed Shell Layer");
	}

	~MyShell()
	{

	}
};
Cherry::Application* Cherry::CreateApplication()
{
    CH_CLIENT_INFO("Returned New Shell");
	return new MyShell();
}