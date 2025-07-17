#include "CHpch.h"
#include "Application.h"
#include <glad/glad.h>
#include "Cherry/Input.h"
#include "KeyCodes.h"

namespace Cherry {

	Application* Application::s_Instance = nullptr;


	// Create the application instance and initialize the window and ImGui layer
	Application::Application()
	{
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);


		//Creating Vertex Array
		m_VertexArray.reset(VertexArray::Create());

		float Vertices[3 * 7] =
		{
			//     Position            |      Color (RGBA)
			-0.5f, -0.5f,  0.0f,        1.0f, 0.5f, 0.0f, 1.0f,  // Left - Orange
			 0.5f, -0.5f,  0.0f,        1.0f, 1.0f, 0.0f, 1.0f,  // Right - Yellow
			 0.0f,  0.5f,  0.0f,        1.0f, 0.0f, 0.0f, 1.0f   // Top - Red
		};

		//Creating Vertex Buffer
		std::shared_ptr<VertexBuffer>m_VertexBuffer;
		m_VertexBuffer.reset(VertexBuffer::Create(Vertices, sizeof(Vertices), GL_STATIC_DRAW));
		//Setting Buffer Layout
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" }
		};
		m_VertexBuffer->SetLayout(layout);




		//Adding Vertex Buffer TO Vertex Array
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		//Creating Index Buffer
		uint32_t Indices[3] = { 0 , 1 , 2 };

		std::shared_ptr<IndexBuffer>m_IndexBuffer;
		m_IndexBuffer.reset(IndexBuffer::Create(Indices, sizeof(Indices) / sizeof(uint32_t), GL_STATIC_DRAW));

		//Adding Index Buffer TO Vertex Array
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		///////
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

		m_SquareVA.reset(VertexArray::Create());

		std::shared_ptr<IndexBuffer>m_SquareIB;

		m_SquareIB.reset(IndexBuffer::Create(SquareIndices, sizeof(SquareIndices) / sizeof(uint32_t), GL_STATIC_DRAW));

		std::shared_ptr<VertexBuffer>m_SquareVB;
		m_SquareVB.reset((VertexBuffer::Create(SquareVertices, sizeof(SquareVertices), GL_STATIC_DRAW)));
		m_SquareVA->SetIndexBuffer(m_SquareIB);


		BufferLayout Squarelayout = {
			{ ShaderDataType::Float3, "a_Position" }
		};
		m_SquareVB->SetLayout(Squarelayout);

		m_SquareVA->AddVertexBuffer(m_SquareVB);

		




		//Shader		--ALOT OF WORK
		std::string vertexSource = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";

		std::string fregmantSource = R"(
			#version 330 core
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;

			}
		)";
		m_Shader.reset(new Shader(vertexSource, fregmantSource));



		///////
		std::string vertexSourceSquare = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
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
		///////
		m_SquareS.reset(new Shader(vertexSourceSquare, fregmantSourceSquare));
	}


	// Destroy the application instance and clean up resources
	Application::~Application()
	{
		CH_CORE_TRACE("Application Destroyed!");
		s_Instance = nullptr;
		CH_CORE_ASSERT(!s_Instance, "Application Already exist!");
	}

	// Initialize the application
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
		CH_CORE_TRACE("Pushed Layer: {0}", layer->GetName());
		CH_CORE_TRACE("Pushed Layer: {0}", m_ImGuiLayer->GetName());

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

		while (m_Running) {

			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			
			// Square
			m_SquareS->Bind();
			m_SquareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffers()->GetCount(), GL_UNSIGNED_INT, nullptr);

			//Triangle
			m_Shader->Bind();
			m_VertexArray->Bind();
			glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffers()->GetCount(), GL_UNSIGNED_INT, nullptr);

			// Update Input
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();



			//Update IMGUI
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();




			// Update Window
			CH_CORE_ASSERT(m_Window, "Window is null!");
			m_Window->OnUpdate();
		}
	}

	// Handle window close event
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}
