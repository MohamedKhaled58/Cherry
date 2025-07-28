#pragma once

#ifdef CH_PLATFORM_WINDOWS

extern Cherry::Application* Cherry::CreateApplication();

int main(int argc, char** argv)
{
	Cherry::Log::Init();

	

	auto App = Cherry::CreateApplication();
	App->Run();


	delete App;
}

#endif // CH_PLATFORM_WINDOWS
