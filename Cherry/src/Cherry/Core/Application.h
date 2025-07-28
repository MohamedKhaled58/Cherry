#pragma once
#include "Core.h"
#include "Window.h"

#include "Cherry/Core/LayerStack.h"
#include "Cherry/Events/Event.h"
#include "Cherry/Events/ApplicationEvent.h"

#include "Cherry/Core/TimeStep.h"

#include "Cherry/ImGui/ImGuiLayer.h"
#include "Cherry/OrthographicCameraController.h"

namespace Cherry {

	class CHERRY_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() {
			if (!m_Window)
				throw std::runtime_error("Window is null");
			return *m_Window;
		}
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		bool m_Minimized = false;
	private:
		static Application* s_Instance;
	};

	// TO BE DEFINED IN CLIENT
	Application* CreateApplication();
}


