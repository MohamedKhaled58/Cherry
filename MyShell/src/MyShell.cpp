#include "Cherry.h"
#include <Platform/OpenGL/OpenGLShader.h>

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Cherry/OrthographicCameraController.h>


class ShellLayer : public Cherry::Layer
{
public:

    ShellLayer()
       : Layer("MyShell"), m_CameraController(1.78f, true)
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
        // Square Setup
        ///////////////////////////
        float SquareVertices[5 * 4] = {
            // x      y      z
            -0.5f, -0.5f, 0.0f,    0.0f,0.0f,             // Bottom-left  (0)
             0.5f, -0.5f, 0.0f,    1.0f,0.0f,                 // Bottom-right (1)
             0.5f,  0.5f, 0.0f,    1.0f,1.0f,                 // Top-right    (2)
            -0.5f,  0.5f, 0.0f,    0.0f,1.0f                  // Top-left     (3)
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
            { Cherry::ShaderDataType::Float3, "a_Position" },
            { Cherry::ShaderDataType::Float2, "a_TexCoord" }
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
        m_Shader = Cherry::Shader::Create("Triangle", vertexSource, fregmantSource);

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
        m_FlatColorShader = Cherry::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);
  
        auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");
        if (!textureShader) {
            CH_CORE_ERROR("Failed to load Texture shader from assets/shaders/Texture.glsl");
            // You might want to create a fallback shader or handle this error
            return; // or create a default shader
        }

        // Debug: Print available shaders
        auto names = m_ShaderLibrary.GetShaderNames();
        CH_CORE_INFO("Loaded {} shaders:", names.size());
        for (const auto& name : names) {
            CH_CORE_INFO("  Available shader: '{}'", name);
        }

        // Load textures
        m_Texture = Cherry::Texture2D::Create("assets/textures/Checkerboard.png");
        m_CherryLogoTexture = Cherry::Texture2D::Create("assets/textures/Cherrylogo.png");

        // Setup texture shader
        if (textureShader) {
            std::dynamic_pointer_cast<Cherry::OpenGLShader>(textureShader)->Bind();
            std::dynamic_pointer_cast<Cherry::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
        }

    }//Shell Layer

    virtual void OnImGuiRender() override
    {
        ImGui::Begin("Camera Controller");

        // Camera Position Controls
        if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 position = m_CameraController.GetCameraPosition();
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);

            float cameraSpeed = m_CameraController.GetCameraSpeed();
            if (ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0.1f, 20.0f))
            {
                m_CameraController.SetCameraSpeed(cameraSpeed);
            }

            if (ImGui::Button("Reset Camera"))
            {
                m_CameraController.Reset();
            }
        }

        // Camera Rotation Controls
        if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float rotation = m_CameraController.GetCameraRotation();
            ImGui::Text("Rotation: %.1f°", rotation);

            float rotationSpeed = m_CameraController.GetRotationSpeed();
            if (ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 10.0f, 360.0f))
            {
                m_CameraController.SetRotationSpeed(rotationSpeed);
            }

            bool rotationEnabled = m_CameraController.IsRotationEnabled();
            if (ImGui::Checkbox("Enable Rotation", &rotationEnabled))
            {
                m_CameraController.SetRotationEnabled(rotationEnabled);
            }
        }

        // Camera Zoom Controls
        if (ImGui::CollapsingHeader("Zoom", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float zoomLevel = m_CameraController.GetZoomLevel();
            float minZoom = m_CameraController.GetMinZoom();
            float maxZoom = m_CameraController.GetMaxZoom();

            if (ImGui::SliderFloat("Zoom Level", &zoomLevel, minZoom, maxZoom))
            {
                m_CameraController.SetZoomLevel(zoomLevel);
            }

            float zoomSpeed = m_CameraController.GetZoomSpeed();
            if (ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.1f, 2.0f))
            {
                m_CameraController.SetZoomSpeed(zoomSpeed);
            }

            // Zoom limits controls
            ImGui::Separator();
            ImGui::Text("Zoom Limits:");
            float newMinZoom = minZoom;
            float newMaxZoom = maxZoom;

            if (ImGui::DragFloat("Min Zoom", &newMinZoom, 0.01f, 0.01f, maxZoom - 0.01f))
            {
                m_CameraController.SetZoomLimits(newMinZoom, maxZoom);
            }

            if (ImGui::DragFloat("Max Zoom", &newMaxZoom, 0.1f, minZoom + 0.01f, 100.0f))
            {
                m_CameraController.SetZoomLimits(minZoom, newMaxZoom);
            }
        }

        // Camera Bounds Controls
        if (ImGui::CollapsingHeader("Camera Bounds"))
        {
            bool boundsEnabled = m_CameraController.IsBoundsEnabled();
            if (ImGui::Checkbox("Enable Bounds", &boundsEnabled))
            {
                m_CameraController.EnableBounds(boundsEnabled);
            }

            if (boundsEnabled)
            {
                glm::vec4 bounds = m_CameraController.GetBounds();
                float left = bounds.x, right = bounds.y;
                float bottom = bounds.z, top = bounds.w;

                bool boundsChanged = false;
                boundsChanged |= ImGui::DragFloat("Left", &left, 0.1f);
                boundsChanged |= ImGui::DragFloat("Right", &right, 0.1f);
                boundsChanged |= ImGui::DragFloat("Bottom", &bottom, 0.1f);
                boundsChanged |= ImGui::DragFloat("Top", &top, 0.1f);

                if (boundsChanged && left < right && bottom < top)
                {
                    m_CameraController.SetBounds(left, right, bottom, top);
                }
            }
        }

        // Mouse Controls
        if (ImGui::CollapsingHeader("Mouse Controls"))
        {
            float mouseSensitivity = m_CameraController.GetMouseSensitivity();
            if (ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.0001f, 0.01f, "%.4f"))
            {
                m_CameraController.SetMouseSensitivity(mouseSensitivity);
            }

            ImGui::Text("Mouse panning: Hold middle mouse button and drag");
        }

        // Camera Information (Read-only)
        if (ImGui::CollapsingHeader("Camera Info"))
        {
            glm::vec3 pos = m_CameraController.GetCameraPosition();
            float rotation = m_CameraController.GetCameraRotation();
            float zoom = m_CameraController.GetZoomLevel();

            ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
            ImGui::Text("Rotation: %.1f°", rotation);
            ImGui::Text("Zoom Level: %.2f", zoom);
            ImGui::Text("Zoom Range: %.2f - %.2f",
                m_CameraController.GetMinZoom(),
                m_CameraController.GetMaxZoom());

            if (m_CameraController.IsBoundsEnabled())
            {
                glm::vec4 bounds = m_CameraController.GetBounds();
                ImGui::Text("Bounds: L:%.1f R:%.1f B:%.1f T:%.1f",
                    bounds.x, bounds.y, bounds.z, bounds.w);
            }
            else
            {
                ImGui::Text("Bounds: Disabled");
            }
        }

        // Input Controls Info
        if (ImGui::CollapsingHeader("Controls"))
        {
            ImGui::Text("Movement:");
            ImGui::BulletText("WASD or Arrow Keys - Move camera");
            ImGui::Text("Rotation:");
            ImGui::BulletText("Q/E - Rotate camera (if enabled)");
            ImGui::Text("Zoom:");
            ImGui::BulletText("Mouse Wheel - Zoom in/out");
        }
        // Controls Reference
        if (ImGui::CollapsingHeader("Controls"))
        {
            ImGui::Text("Movement:");
            ImGui::BulletText("WASD or Arrow Keys - Move camera");
            ImGui::Text("Rotation:");
            ImGui::BulletText("Q/E - Rotate camera (if enabled)");
            ImGui::Text("Zoom:");
            ImGui::BulletText("Mouse Wheel - Zoom in/out");
            ImGui::Text("Mouse:");
            ImGui::BulletText("Middle Mouse + Drag - Pan camera");
        }

        ImGui::End();

        // Keep your existing settings window
        ImGui::Begin("Render Settings");
        ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }

    void OnUpdate(Cherry::TimeStep timeStep) override
    {
        // Update the camera controller (handles all input and camera movement)
        m_CameraController.OnUpdate(timeStep);

        // Clear screen
        Cherry::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Cherry::RenderCommand::Clear();

        // Use the camera from the controller instead of your manual camera
        Cherry::Renderer::BeginScene(m_CameraController.GetCamera());

        // Remove these lines - the controller handles camera position/rotation now:
        // m_Camera.SetPosition(m_CameraPosition);
        // m_Camera.SetRotation(m_CameraRotation);

        // Rest of your rendering code stays the same...
        glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
        glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

        std::dynamic_pointer_cast<Cherry::OpenGLShader>(m_FlatColorShader)->Bind();
        std::dynamic_pointer_cast<Cherry::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

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

        auto textureShader = m_ShaderLibrary.Get("texture");
        if (textureShader) {
            m_Texture->Bind();
            Cherry::Renderer::Submit(textureShader, m_FlatColorVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
            m_CherryLogoTexture->Bind();
            Cherry::Renderer::Submit(textureShader, m_FlatColorVertexArray);
        }
        else {
            CH_CORE_WARN("Texture shader not found! Available shaders:");
            auto names = m_ShaderLibrary.GetShaderNames();
            for (const auto& name : names) {
                CH_CORE_WARN("  '{}'", name);
            }
        }

        Cherry::Renderer::EndScene();
    }

    void OnEvent(Cherry::Event& e) override
    {
        m_CameraController.OnEvent(e);
    }

    private:
        Cherry::ShaderLibrary m_ShaderLibrary;
        REF(Cherry::Shader) m_Shader;
        REF(Cherry::VertexArray) m_VertexArray;

        REF(Cherry::Shader) m_FlatColorShader;
        REF(Cherry::VertexArray) m_FlatColorVertexArray;

        REF(Cherry::Texture2D) m_Texture, m_CherryLogoTexture;

        Cherry::OrthographicCameraController m_CameraController;

        glm::vec3 m_SquareColor = { 0.3f,0.1f,0.8f };
};


class MyShell : public Cherry::Application
{
public:
    MyShell()
    {
        auto* shellLayer = new ShellLayer();
        PushLayer(shellLayer);
        CH_CLIENT_INFO("Pushed Layer: {0}", shellLayer->GetName());
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