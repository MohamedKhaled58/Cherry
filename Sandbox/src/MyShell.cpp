#include "Cherry.h"

class ExampleLayer : public Cherry::Layer
{
public:

	ExampleLayer()
		:Layer("Example") { }
	
	void OnUpdate() override
	{
		CH_CLIENT_INFO("ExampleLayer:Update");
	}
	void OnEvent(Cherry::Event& event) override
	{
		CH_CLIENT_TRACE("{0}",event.ToString());

	}
private:

};


class Sandbox : public Cherry::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer);
		PushOverlay(new Cherry::ImGuiLayer());
	}
	~Sandbox()
	{

	}
private:

};
Cherry::Application* Cherry::CreateApplication()
{
	return new Sandbox();
}