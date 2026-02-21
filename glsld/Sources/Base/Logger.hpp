#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace glsld {
    struct LogSinkConfig {
        std::string               type;
        std::string               pattern;
        std::string               filename;
        spdlog::level::level_enum level;

        bool operator==(const LogSinkConfig& other) const;
    };

    struct LoggerConfig {
        std::string                name;
        std::string                pattern;
        std::vector<LogSinkConfig> sinks;
        spdlog::level::level_enum  level{ spdlog::level::trace };

        bool operator==(const LoggerConfig& other) const;
    };

    class LoggerManager {
    public:
        void Initialize();
        std::shared_ptr<spdlog::logger> GetLogger(std::string_view name);
        std::shared_ptr<spdlog::logger> GetRoot();

        static LoggerManager& GetInstance();

    private:
        LoggerManager();

        void OnConfigChange(const std::unordered_set<LoggerConfig>& old_configs,
                            const std::unordered_set<LoggerConfig>& new_configs);

        void ApplyConfig(const LoggerConfig& config);

        bool initialized_{ false };
    };
}

namespace std {
    template <>
    struct hash<glsld::LogSinkConfig> {
        size_t operator()(const glsld::LogSinkConfig& config) const;
    };

    template <>
    struct hash<glsld::LoggerConfig> {
        size_t operator()(const glsld::LoggerConfig& config) const;
    };
} // namespace std;

namespace YAML {
    template <>
    struct convert<spdlog::level::level_enum> {
        static Node encode(const spdlog::level::level_enum& rhs);
        static bool decode(const Node& node, spdlog::level::level_enum& rhs);
    };

    template <>
    struct convert<glsld::LogSinkConfig> {
        static Node encode(const glsld::LogSinkConfig& rhs);
        static bool decode(const Node& node, glsld::LogSinkConfig& rhs);
    };

    template <>
    struct convert<glsld::LoggerConfig> {
        static Node encode(const glsld::LoggerConfig& rhs);
        static bool decode(const Node& node, glsld::LoggerConfig& rhs);
    };
} // namespace YAML

#include "Logger.inl"

#define GLSLD_LOG_NAME(name) ::glsld::LoggerManager::GetInstance().GetLogger(name)
#define GLSLD_LOG_ROOT()     ::glsld::LoggerManager::GetInstance().GetRoot()

#define GLSLD_LOG_CRITICAL(logger, ...)              \
    if (logger->should_log(spdlog::level::critical)) \
        logger->critical(__VA_ARGS__)

#define GLSLD_LOG_DEBUG(logger, ...)                 \
    if (logger->should_log(spdlog::level::debug))    \
        logger->debug(__VA_ARGS__)

#define GLSLD_LOG_ERROR(logger, ...)                 \
    if (logger->should_log(spdlog::level::err))      \
        logger->error(__VA_ARGS__)

#define GLSLD_LOG_INFO(logger, ...)                  \
    if (logger->should_log(spdlog::level::info))     \
        logger->info(__VA_ARGS__)

#define GLSLD_LOG_TRACE(logger, ...)                 \
    if (logger->should_log(spdlog::level::trace))    \
        logger->trace(__VA_ARGS__)

#define GLSLD_LOG_WARN(logger, ...)                  \
    if (logger->should_log(spdlog::level::warn))     \
        logger->warn(__VA_ARGS__)
