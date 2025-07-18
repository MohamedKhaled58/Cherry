#include "CHpch.h"
#include "Application.h"
#include "Cherry/Renderer/Buffer.h"
#include "Cherry/Renderer/Renderer.h"
#include <glad/glad.h>

namespace Cherry {

    Application* Application::s_Instance = nullptr;


    void Application::UpdateGridForCamera() {
        float left = m_Camera.GetLeft();
        float right = m_Camera.GetRight();
        float bottom = m_Camera.GetBottom();
        float top = m_Camera.GetTop();

        // Calculate grid spacing
        float width = right - left;
        float height = top - bottom;
        float baseSpacing = 0.1f;
        float zoomLevel = width / 4.0f;
        float gridSpacing = baseSpacing * zoomLevel;
        gridSpacing = std::max(0.01f, std::min(gridSpacing, 2.0f));

        // Smart grid spacing for nice numbers
        float logSpacing = std::log10(gridSpacing);
        float powerOf10 = std::pow(10, std::floor(logSpacing));
        float normalizedSpacing = gridSpacing / powerOf10;

        if (normalizedSpacing <= 2.0f) {
            gridSpacing = powerOf10;
        }
        else if (normalizedSpacing <= 5.0f) {
            gridSpacing = 2.0f * powerOf10;
        }
        else {
            gridSpacing = 5.0f * powerOf10;
        }

        std::vector<float> gridVertices;
        std::vector<uint32_t> gridIndices;
        uint32_t vertexIndex = 0;
        m_GridLabels.clear();

        // Grid bounds with padding
        float padding = gridSpacing * 2.0f;
        float gridLeft = left - padding;
        float gridRight = right + padding;
        float gridBottom = bottom - padding;
        float gridTop = top + padding;

        float startX = std::floor(gridLeft / gridSpacing) * gridSpacing;
        float startY = std::floor(gridBottom / gridSpacing) * gridSpacing;

        // Create vertical lines with labels
        for (float x = startX; x <= gridRight; x += gridSpacing) {
            if (x < gridLeft) continue;

            // Grid line vertices
            gridVertices.push_back(x);
            gridVertices.push_back(gridBottom);
            gridVertices.push_back(0.0f);
            gridVertices.push_back(x);
            gridVertices.push_back(gridTop);
            gridVertices.push_back(0.0f);
            gridIndices.push_back(vertexIndex++);
            gridIndices.push_back(vertexIndex++);

            // Add label (only for lines within visible area and not too close to origin)
            if (x >= left && x <= right && std::abs(x) > gridSpacing * 0.1f) {
                GridLabel label;

                // Format number properly
                if (std::abs(x) < 0.001f) {
                    label.text = "0";
                }
                else if (gridSpacing >= 1.0f) {
                    label.text = std::to_string(static_cast<int>(std::round(x)));
                }
                else {
                    // Use ostringstream for better control
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(1) << x;
                    label.text = oss.str();
                    // Remove .0 if present
                    if (label.text.substr(label.text.length() - 2) == ".0") {
                        label.text = label.text.substr(0, label.text.length() - 2);
                    }
                }

                // Position at bottom edge of screen with offset
                label.position = glm::vec3(x, bottom + height * 0.05f, 0.0f);
                label.isHorizontal = true;
                m_GridLabels.push_back(label);
            }
        }

        // Create horizontal lines with labels
        for (float y = startY; y <= gridTop; y += gridSpacing) {
            if (y < gridBottom) continue;

            // Grid line vertices
            gridVertices.push_back(gridLeft);
            gridVertices.push_back(y);
            gridVertices.push_back(0.0f);
            gridVertices.push_back(gridRight);
            gridVertices.push_back(y);
            gridVertices.push_back(0.0f);
            gridIndices.push_back(vertexIndex++);
            gridIndices.push_back(vertexIndex++);

            // Add label (only for lines within visible area and not too close to origin)
            if (y >= bottom && y <= top && std::abs(y) > gridSpacing * 0.1f) {
                GridLabel label;

                // Format number properly
                if (std::abs(y) < 0.001f) {
                    label.text = "0";
                }
                else if (gridSpacing >= 1.0f) {
                    label.text = std::to_string(static_cast<int>(std::round(y)));
                }
                else {
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(1) << y;
                    label.text = oss.str();
                    // Remove .0 if present
                    if (label.text.substr(label.text.length() - 2) == ".0") {
                        label.text = label.text.substr(0, label.text.length() - 2);
                    }
                }

                // Position at left edge of screen with offset
                label.position = glm::vec3(left + width * 0.03f, y, 0.0f);
                label.isHorizontal = false;
                m_GridLabels.push_back(label);
            }
        }

        // Add origin labels if origin is visible
        if (left <= 0.0f && right >= 0.0f && bottom <= 0.0f && top >= 0.0f) {
            // Add "0" at origin
            GridLabel originLabel;
            originLabel.text = "0";
            originLabel.position = glm::vec3(left + width * 0.01f, bottom + height * 0.01f, 0.0f);
            originLabel.isHorizontal = false;
            m_GridLabels.push_back(originLabel);
        }

        // Update grid buffers
        m_GridVertexArray.reset(VertexArray::Create());
        std::shared_ptr<VertexBuffer> gridVertexBuffer;
        gridVertexBuffer.reset(VertexBuffer::Create(gridVertices.data(), sizeof(float) * gridVertices.size()));
        BufferLayout gridLayout = { { ShaderDataType::Float3, "a_Position" } };
        gridVertexBuffer->SetLayout(gridLayout);
        m_GridVertexArray->AddVertexBuffer(gridVertexBuffer);
        std::shared_ptr<IndexBuffer> gridIndexBuffer;
        gridIndexBuffer.reset(IndexBuffer::Create(gridIndices.data(), gridIndices.size()));
        m_GridVertexArray->SetIndexBuffer(gridIndexBuffer);
        m_GridNeedsUpdate = false;

        CH_CORE_TRACE("Grid updated: {0} labels created", m_GridLabels.size());
    }

    Application::Application()
        :m_Camera(-1.6f, 1.6f, -0.9f, 0.9f, -1.0f, 1.0f)
    {
        s_Instance = this;
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        // Initialize text rendering first


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
        m_VertexArray.reset(VertexArray::Create());

        // Creating Vertex Buffer
        std::shared_ptr<VertexBuffer>m_VertexBuffer;
        m_VertexBuffer.reset(VertexBuffer::Create(Vertices, sizeof(Vertices)));

        // Setting Buffer Layout
        BufferLayout layout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" }
        };
        m_VertexBuffer->SetLayout(layout);

        // Adding Vertex Buffer TO Vertex Array
        m_VertexArray->AddVertexBuffer(m_VertexBuffer);

        // Creating Index Buffer
        std::shared_ptr<IndexBuffer>m_IndexBuffer;
        m_IndexBuffer.reset(IndexBuffer::Create(Indices, sizeof(Indices) / sizeof(uint32_t)));

        // Adding Index Buffer TO Vertex Array
        m_VertexArray->SetIndexBuffer(m_IndexBuffer);

        ///////////////////////////
        // Square Setup - FIXED
        ///////////////////////////
        float SquareVertices[3 * 4] = {
            // x      y      z
            -0.75f, -0.75f, 0.0f,  // Bottom-left  (0)
             0.75f, -0.75f, 0.0f,  // Bottom-right (1)
             0.75f,  0.75f, 0.0f,  // Top-right    (2)
            -0.75f,  0.75f, 0.0f   // Top-left     (3)
        };
        uint32_t SquareIndices[6] = {
            0, 1, 2,  // Triangle 1
            2, 3, 0   // Triangle 2
        };

        /////////Setting Up Square//////////
        m_SquareVA.reset(VertexArray::Create());

        // Creating Vertex Buffer
        std::shared_ptr<VertexBuffer>m_SquareVB;
        m_SquareVB.reset((VertexBuffer::Create(SquareVertices, sizeof(SquareVertices))));
        BufferLayout Squarelayout = {
            { ShaderDataType::Float3, "a_Position" }
        };
        m_SquareVB->SetLayout(Squarelayout);
        m_SquareVA->AddVertexBuffer(m_SquareVB);

        // Creating Index Buffer - FIXED: Actually add it to vertex array
        std::shared_ptr<IndexBuffer>m_SquareIB;
        m_SquareIB.reset(IndexBuffer::Create(SquareIndices, sizeof(SquareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(m_SquareIB);  // THIS WAS MISSING!

        // Initialize grid vertex array (but don't populate it yet)
        m_GridVertexArray.reset(VertexArray::Create());

        // Create grid shader - this version works with ViewProjection matrix
        std::string gridVertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        uniform mat4 u_ViewProjection;
        void main()
        {
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
        }
    )";

        std::string gridFragmentSource = R"(
        #version 330 core
        out vec4 color;
        void main()
        {
            color = vec4(0.3, 0.3, 0.3, 1.0);  // Gray color for grid
        }
    )";
        m_GridShader.reset(new Shader(gridVertexSource, gridFragmentSource));

        // Enable grid labels by default
        m_ShowGridLabels = true;

        // Generate initial grid
        UpdateGridForCamera();

        ///////////////////////////
        // Triangle Shader
        ///////////////////////////
        std::string vertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        uniform mat4 u_ViewProjection;

        out vec3 v_Position;
        out vec4 v_Color;

        void main()
        {
            v_Position = a_Position;
            v_Color = a_Color;
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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
        m_Shader.reset(new Shader(vertexSource, fregmantSource));

        ///////////////////////////
        // Square Shader
        ///////////////////////////
        std::string vertexSourceSquare = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;

        out vec3 v_Position;
        uniform mat4 u_ViewProjection;
        void main()
        {
            v_Position = a_Position;
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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
        m_SquareS.reset(new Shader(vertexSourceSquare, fregmantSourceSquare));

        CH_CORE_INFO("Application initialized successfully");
    }

    // Destroy the application instance and clean up resources
    Application::~Application()
    {
        CH_CORE_TRACE("Application Destroyed!");
 
        s_Instance = nullptr;
    }

    // Initialize the application
    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
        CH_CORE_TRACE("Pushed Layer: {0}", layer->GetName());
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
        CH_CORE_TRACE("Pushed Overlay: {0}", layer->GetName());
    }

    // Handle events in the application
    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        // Dispatch event to layers
        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
            (*--it)->OnEvent(e);
            if (e.Handled) break;
        }
        // Handle window events
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
    }

    // Main loop of the application
    void Application::Run()
    {
        static glm::vec3 lastCameraPos = m_Camera.GetPosition();
        static float lastCameraRotation = m_Camera.GetRotation();


        while (m_Running) {
            // Check if camera changed
            if (m_Camera.GetPosition() != lastCameraPos ||
                m_Camera.GetRotation() != lastCameraRotation) {
                m_GridNeedsUpdate = true;
                lastCameraPos = m_Camera.GetPosition();
                lastCameraRotation = m_Camera.GetRotation();
            }

            // Update grid if needed
            if (m_GridNeedsUpdate) {
                UpdateGridForCamera();
            }

            // Clear screen
            RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
            RenderCommand::Clear();

            m_Camera.SetPosition({ 0.5f,0.5f,0.0f });
            m_Camera.SetRotation(45.0f);
            Renderer::BeginScene(m_Camera);

            // Render grid lines
            if (m_GridVertexArray && m_GridShader && m_GridVertexArray->GetIndexBuffers()) {
                m_GridVertexArray->Bind();
                m_GridShader->Bind();
                m_GridShader->UploadUniformMat4("u_ViewProjection", m_Camera.GetViewProjectionMatrix());
                uint32_t indexCount = m_GridVertexArray->GetIndexBuffers()->GetCount();
                if (indexCount > 0) {
                    glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
                }
            }

            // Render square with safety checks
            if (m_SquareVA && m_SquareS && m_SquareVA->GetIndexBuffers()) {
                Renderer::Submit(m_SquareS,m_SquareVA);
            }

            // Render triangle with safety checks
            if (m_VertexArray && m_Shader && m_VertexArray->GetIndexBuffers()) {
                Renderer::Submit(m_Shader,m_VertexArray);
            }

            Renderer::EndScene();

            // Update layers
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate();

            // Update IMGUI
            if (m_ImGuiLayer) {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

            // Update Window
            CH_CORE_ASSERT(m_Window, "Window is null!");
            if (m_Window) {
                m_Window->OnUpdate();
            }
        }
    }

    // Handle window close event
    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }
}