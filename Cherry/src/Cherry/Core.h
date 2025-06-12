#pragma once
#ifdef CH_PLATFORM_WINDOWS
	#ifdef CH_BUILD_DLL
		#define CHERRY_API __declspec(dllexport)
	#else
		#define CHERRY_API __declspec(dllimport)
	#endif // CH_BUILD_DLL
#else
#error CHERRY ONLY SUPPORTS WINDOWS
#endif // CH_PLATFORM_WINDOWS
