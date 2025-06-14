#pragma once
#include "Cherry/Core.h"
#include "Layer.h"
#include <vector>
namespace Cherry {
	class CHERRY_API LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::const_iterator begin()  { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end()  { return m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::iterator m_LayerInsert;
	};
}