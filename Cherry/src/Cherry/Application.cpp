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
		CH_CORE_ASSERT(!s_Instance, "Application Already exist!");
		s_Instance = this;
		
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

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
		CH_CORE_ASSERT(m_Window, "Window is null!");
		while (m_Running) {

			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
		

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
