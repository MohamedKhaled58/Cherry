#pragma once
#ifdef CH_PLATFORM_WINDOWS
	#ifdef CH_DYNAMIC_LINKING
		#ifdef CH_BUILD_DLL
			#define CHERRY_API __declspec(dllexport)
		#else
			#define CHERRY_API __declspec(dllimport)
		#endif // CH_BUILD_DLL
	#else
		#define CHERRY_API
	#endif // CH_DYNAMIC_LINKING
#else
	#error CHERRY ONLY SUPPORTS WINDOWS
#endif // CH_PLATFORM_WINDOWS

#ifdef CH_DEBUG
#define CH_ENABLE_ASSERTS
#endif

#ifdef CH_ENABLE_ASSERTS
#define CH_CLIENT_ASSERT(x, ...) { if(!(x)) { CH_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#define CH_CORE_ASSERT(x, ...) { if(!(x)) { CH_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define CH_CLIENT_ASSERT(x, ...)
#define CH_CORE_ASSERT(x, ...)
#endif



#define BIT(X) (1<<X)
#define CH_BIND_EVENT_FN(fn) (std::bind(&fn,this,std::placeholders::_1))
