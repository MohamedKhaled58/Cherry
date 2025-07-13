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

		float Vertices[3 * 3] =
		{
			-0.5f,	-0.5f,	0.0f,		//LEFT POINT
			 0.5f,	-0.5f,	0.0f,		//RIGHT PONT
			 0.0f,	 0.5f,	0.0f		//TOP POINT
		};

		//	Vertex Array
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		//	Vertex Buffer
		m_VertexBuffer.reset(VertexBuffer::Create(Vertices, sizeof(Vertices), GL_STATIC_DRAW));

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		//	Index Buffer
		uint32_t Indices[3] = { 0 , 1 , 2 };
		m_IndexBuffer.reset(IndexBuffer::Create(Indices, sizeof(Indices) / sizeof(uint32_t), GL_STATIC_DRAW));


		//	Shader		--ALOT OF WORK
		std::string vertexSource = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;
			void main()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";

		std::string fregmantSource = R"(
			#version 330 core
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
			}
		)";

		m_Shader.reset(new Shader(vertexSource,fregmantSource));
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
		
			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

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
