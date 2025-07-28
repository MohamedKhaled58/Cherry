#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Core/TimeStep.h"
#include "Cherry/Events/Event.h"

namespace Cherry {
	class CHERRY_API Layer 
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach(){}
		virtual void OnDetach(){}
		virtual void OnUpdate(TimeStep timeStep){}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event){}

		inline const std::string GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;

	};
}