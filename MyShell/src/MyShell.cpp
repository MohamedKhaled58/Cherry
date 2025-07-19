#include "Cherry.h"

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>


class ShellLayer : public Cherry::Layer
{
public:

    ShellLayer()
        :Layer("Shell"),m_Camera(-1.6f, 1.6f, -0.9f, 0.9f, -1.0f, 1.0f), m_CameraPosition({0.0f,0.0f,0.0f}), m_CameraRotation(0.0f), m_SquarePosition(0.0f)
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
        std::shared_ptr<Cherry::VertexBuffer>m_VertexBuffer;
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
        std::shared_ptr<Cherry::IndexBuffer>m_IndexBuffer;
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
        m_SquareVA.reset(Cherry::VertexArray::Create());

        // Creating Vertex Buffer
        std::shared_ptr<Cherry::VertexBuffer>m_SquareVB;
        m_SquareVB.reset((Cherry::VertexBuffer::Create(SquareVertices, sizeof(SquareVertices))));
        Cherry::BufferLayout Squarelayout = {
            { Cherry::ShaderDataType::Float3, "a_Position" }
        };
        m_SquareVB->SetLayout(Squarelayout);
        m_SquareVA->AddVertexBuffer(m_SquareVB);

        // Creating Index Buffer - FIXED: Actually add it to vertex array
        std::shared_ptr<Cherry::IndexBuffer>m_SquareIB;
        m_SquareIB.reset(Cherry::IndexBuffer::Create(SquareIndices, sizeof(SquareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(m_SquareIB);


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
        m_Shader.reset(new Cherry::Shader(vertexSource, fregmantSource));

        ///////////////////////////
        // Square Shader
        ///////////////////////////
        std::string vertexSourceSquare = R"(
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

        std::string fregmantSourceSquare = R"(
        #version 330 core
        layout(location = 0) out vec4 color;

        in vec3 v_Position;

        void main()
        {
            color = vec4(v_Position * 0.5 + 0.5, 1.0);
        }
    )";
        m_SquareS.reset(new Cherry::Shader(vertexSourceSquare, fregmantSourceSquare));
    }

    virtual void OnImGuiRender() override
    {

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


        if (Cherry::Input::IsKeyPressed(CH_KEY_J))
            m_SquarePosition.x -= m_SquareMoveSpeed * timeStep;
        else if (Cherry::Input::IsKeyPressed(CH_KEY_L))
            m_SquarePosition.x += m_SquareMoveSpeed * timeStep;

        if (Cherry::Input::IsKeyPressed(CH_KEY_I))
            m_SquarePosition.y += m_SquareMoveSpeed * timeStep;
        else if (Cherry::Input::IsKeyPressed(CH_KEY_K))
            m_SquarePosition.y -= m_SquareMoveSpeed * timeStep;

        // Clear screen
        Cherry::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Cherry::RenderCommand::Clear();

        m_Camera.SetPosition(m_CameraPosition);
        m_Camera.SetRotation(m_CameraRotation);
        Cherry::Renderer::BeginScene(m_Camera);

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        for (int y = 0; y < 20; y++)
        {
            for (int x = 0; x < 20; x++)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
                Cherry::Renderer::Submit(m_SquareS, m_SquareVA, transform);

            }
        }
       

        //Cherry::Renderer::Submit(m_Shader, m_VertexArray);

        Cherry::Renderer::EndScene();
    }

    void OnEvent(Cherry::Event& event) override
    {
    }

    private:
        std::shared_ptr<Cherry::Shader> m_Shader;
        std::shared_ptr<Cherry::VertexArray> m_VertexArray;

        std::shared_ptr<Cherry::Shader> m_SquareS;
        std::shared_ptr<Cherry::VertexArray> m_SquareVA;
        Cherry::OrthographicCamera m_Camera;

        glm::vec3 m_CameraPosition;
        float m_CameraRotation;

        float m_CameraMoveSpeed = 1.0f;
        float m_CameraRotationSpeed = 90.0f;

        glm::vec3 m_SquarePosition;
        float m_SquareMoveSpeed = 1.0f;
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
private:

};
Cherry::Application* Cherry::CreateApplication()
{
    CH_CLIENT_INFO("Returned New Shell");
	return new MyShell();
}