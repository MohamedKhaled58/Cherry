#include "Application.h"

#include "Cherry/Log.h"
#include "Cherry/Events/ApplicationEvent.h"

namespace Cherry {
	Application::Application()
	{
	}
	Application::~Application()
	{
	}
	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			CH_CLIENT_INFO(e.ToString());
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			CH_CLIENT_TRACE(e.ToString());
		}
		while (true);
	}
}
