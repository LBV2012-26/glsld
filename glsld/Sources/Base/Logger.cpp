#include "pch.hpp"
#include "Logger.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include "Utils/Utils.hpp"

namespace glsld {
    Logger::Logger() {
        if (initialized_) {
            return;
        }

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            utils::GetFilePath("Win64/glsld.log"), true);

        auto logger = std::make_shared<spdlog::logger>("glsld", sink);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->set_level(spdlog::level::trace);
        spdlog::register_logger(logger);

        initialized_ = true;
    }

    Logger& Logger::GetInstance() {
        static Logger instance;
        return instance;
    }
}
