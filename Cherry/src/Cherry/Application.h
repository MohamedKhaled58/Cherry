#pragma once
#include "Core.h"
#include "Cherry/Events/Event.h"
#include "Cherry/Events/ApplicationEvent.h"
#include "Window.h"


namespace Cherry {

	class CHERRY_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// TO BE DEFINED IN CLIENT
	Application* CreateApplication();
}


