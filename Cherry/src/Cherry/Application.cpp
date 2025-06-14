#include "CHpch.h"
#include "Application.h"
#include <glad/glad.h>
#include "Cherry/Input.h"
#include "KeyCodes.h"

namespace Cherry {

	 Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		CH_CORE_ASSERT(!s_Instance, "Application Already exist!");
		m_Window = std::unique_ptr<Window>(Window::Create());
		if (!m_Window) {
			throw std::runtime_error("Failed to create Window");
		}
		s_Instance = this;
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

	}
	Application::~Application()
	{
	}

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
	void Application::Run()
	{
		
		while (m_Running) {
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
			CH_CORE_ASSERT(m_Window, "Window is null!");
			m_Window->OnUpdate();


			



		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}
