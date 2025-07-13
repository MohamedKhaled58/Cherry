#include "Cherry.h"

#include "imgui/imgui.h"

class ExampleLayer : public Cherry::Layer
{
public:

	ExampleLayer() :Layer("Example") 
	{

	}

	virtual void OnImGuiRender() override
	{

	}

	void OnUpdate() override
	{
		// Poll input events
		if (Cherry::Input::IsKeyPressed(CH_KEY_ESCAPE)) {
			auto result = MessageBoxA(nullptr, "Are you sure you want to exit?", "Exit Confirmation", MB_YESNO | MB_ICONQUESTION);
			switch (result) {
			case IDYES:
				CH_CLIENT_INFO("Exiting application...");
				AppendMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, 0, 0);
				PostQuitMessage(0);
				CH_CLIENT_INFO("Application exited successfully.");
				// Optionally, you can also call exit(0) to terminate the application immediately
				exit(0);
				break;
			case IDNO:
				CH_CLIENT_INFO("Continuing application...");
				break;
			default:
				CH_CLIENT_ERROR("Unexpected result from MessageBeep: {0}", result);
				break;
			}
		}
	}
	void OnEvent(Cherry::Event& event) override
	{
		if (event.GetEventType() == Cherry::EventType::KeyPressed) {
			Cherry::KeyPressedEvent& keyEvent = (Cherry::KeyPressedEvent&)event;
			//CH_CLIENT_TRACE("{0}",(char)keyEvent.GetKeyCode());
		}
	}
};


class Sandbox : public Cherry::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer);
		CH_CLIENT_WARN("Sandbox");
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