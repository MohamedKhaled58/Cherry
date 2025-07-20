#include "CHpch.h"
#include <spdlog/sinks/basic_file_sink.h>

namespace Cherry {
	REF(spdlog::logger) Log::s_CoreLogger;
	REF(spdlog::logger) Log::s_ClientLogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("[CHERRY]");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("[CLIENT]");
		s_ClientLogger->set_level(spdlog::level::trace);

		// File sink to log to a file
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
		s_CoreLogger->sinks().push_back(file_sink);
		s_ClientLogger->sinks().push_back(file_sink);

		CH_CORE_WARN("CHERRY Initialized Log With (7) Levels");
	}
}