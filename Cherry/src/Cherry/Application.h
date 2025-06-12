#pragma once
#include "Core.h"

namespace Cherry {
	class CHERRY_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();
	};

	// TO BE DEFINED IN CLIENT
	Application* CreateApplication();
}


