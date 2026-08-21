#pragma once

#include <spdlog/spdlog.h>

namespace glsld {
    class Logger {
    public:
        static Logger& GetInstance();

    private:
        Logger();

        bool initialized_{ false };
    };
}

#define GLSLD_LOG(log_level, ...)                                                \
    do {                                                                         \
        auto logger = spdlog::get("glsld");                                      \
        if (logger != nullptr && logger->should_log(spdlog::level::log_level)) { \
            logger->log(spdlog::level::log_level, __VA_ARGS__);                  \
        }                                                                        \
    } while (false)
