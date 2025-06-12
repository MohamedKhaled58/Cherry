#pragma once

#ifdef CH_PLATFORM_WINDOWS

extern Cherry::Application* Cherry::CreateApplication();

int main(int argc, char** argv)
{
	Cherry::Log::Init();

	CH_CORE_WARN("Initialized Log");
	int a = 5;
	CH_CLIENT_INFO("Var = {0}",a);
	CH_CLIENT_CRIT("Hello FROM CLIENT");

	auto App = Cherry::CreateApplication();
	App->Run();


	delete App;
}

#endif // CH_PLATFORM_WINDOWS
