#pragma once
#include "Cherry/Core/Layer.h"
#include <Cherry/Events/MouseEvent.h>
#include <Cherry/Events/KeyEvent.h>
#include <Cherry/Events/ApplicationEvent.h>

namespace Cherry {
	class CHERRY_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;		//Must Mark as override to avoid warning C4250
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_time = 0.0f;
	};
}