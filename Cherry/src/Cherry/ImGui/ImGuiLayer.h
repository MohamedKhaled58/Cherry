#pragma once

#include "Cherry/Layer.h"
#include <Cherry/Events/MouseEvent.h>
#include <Cherry/Events/KeyEvent.h>
#include <Cherry/Events/ApplicationEvent.h>

namespace Cherry {
	class CHERRY_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);
	private:
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event);
		bool OnMouseMovedEvent(MouseMovedEvent& event);
		bool OnMouseScrolledEvent(MouseScrolledEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnKeyTypedEvent(KeyTypedEvent& event);
		bool OnKeyReleasedEvent(KeyReleasedEvent& event);
		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnWindowCloseEvent(WindowCloseEvent& event);

	private:
		float m_time = 0.0f;
	};
}