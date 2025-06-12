#include "Cherry.h"

class Sandbox : public Cherry::Application
{
public:
	Sandbox()
	{

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