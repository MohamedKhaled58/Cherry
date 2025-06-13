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

#ifdef CH_ENABLE_ASSERTS
	#define CH_CLIENT_ASSERT(x, ...) {if(!(x)) {CH_CLIENT_ERROR("Assertion Failed: {0}",__VA__ARGS__);__debugbreak();}}
	#define CH_CORE_ASSERT(x, ...) {if(!(x)) {CH_CORE_ERROR("Assertion Failed: {0}",__VA__ARGS__);__debugbreak();}}
#else
	#define CH_CLIENT_ASSERT(x, ...)
	#define CH_CORE_ASSERT(x, ...)
#endif // CH_ENABLE_ASSERTS


#define BIT(X) (1<<X)
