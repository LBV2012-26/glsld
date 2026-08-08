#include "pch.hpp"
#include "Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace glsld {
    Logger::Logger() {
        if (initialized_) {
            return;
        }

        auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
#ifdef _WIN64
        sink->set_color(spdlog::level::trace, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#else
        sink->set_color(spdlog::level::trace, "\033[34m"); // Blue
#endif

        auto logger = std::make_shared<spdlog::logger>("glsld", sink);
        logger->set_pattern("[%T][%^%l%$] %n: %v");
#ifdef _DEBUG
        logger->set_level(spdlog::level::trace);
#else
        logger->set_level(spdlog::level::info);
#endif
        spdlog::register_logger(logger);

        initialized_ = true;
    }

    Logger& Logger::GetInstance() {
        static Logger instance;
        return instance;
    }
}
