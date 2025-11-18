#pragma once
#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

#pragma warning(push)
#pragma warning(disable : 4251)


namespace Cherry {
	class CHERRY_API Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr < spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr < spdlog::logger> s_CoreLogger;
		static std::shared_ptr < spdlog::logger> s_ClientLogger;
	};
}
//CORE LOG MACROS
#define CH_CORE_TRACE(...)	::Cherry::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CH_CORE_INFO(...)	::Cherry::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CH_CORE_DEBUG(...)	::Cherry::Log::GetCoreLogger()->debug(__VA_ARGS__)
#define CH_CORE_WARN(...)	::Cherry::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CH_CORE_ERROR(...)	::Cherry::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CH_CORE_CRIT(...)	::Cherry::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define CH_CORE_OFF(...)	::Cherry::Log::GetCoreLogger()->off(__VA_ARGS__)

//CLIENT LOG MACROS
#define CH_CLIENT_TRACE(...)	::Cherry::Log::GetClientLogger()->trace(__VA_ARGS__)
#define CH_CLIENT_INFO(...)		::Cherry::Log::GetClientLogger()->info(__VA_ARGS__)
#define CH_CLIENT_DEBUG(...)	::Cherry::Log::GetClientLogger()->debug(__VA_ARGS__)
#define CH_CLIENT_WARN(...)		::Cherry::Log::GetClientLogger()->warn(__VA_ARGS__)
#define CH_CLIENT_ERROR(...)	::Cherry::Log::GetClientLogger()->error(__VA_ARGS__)
#define CH_CLIENT_CRIT(...)		::Cherry::Log::GetClientLogger()->critical(__VA_ARGS__)
#define CH_CLIENT_OFF(...)		::Cherry::Log::GetClientLogger()->off(__VA_ARGS__)



#pragma warning(pop)
