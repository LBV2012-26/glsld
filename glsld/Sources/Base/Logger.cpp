#include "stdafx.h"
#include "logger.hpp"

#include <algorithm>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Base/Config.hpp"
#include "Base/Hash.hpp"
#include "Base/YamlTypes.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    bool LoggerConfig::operator==(const LoggerConfig& other) const {
        if (name != other.name || pattern != other.pattern || level != other.level || sinks.size() != other.sinks.size()) {
            return false;
        }

        std::unordered_multiset<LogSinkConfig> this_sink_set(sinks.begin(), sinks.end());

        for (const auto& other_sink : other.sinks) {
            auto it = this_sink_set.find(other_sink);
            if (it == this_sink_set.end()) {
                return false;
            }

            this_sink_set.erase(it);
        }

        return this_sink_set.empty();
    }

    LoggerManager::LoggerManager() {
        if (!spdlog::get("root")) {
            auto root_logger = spdlog::stdout_color_mt("root");
            root_logger->set_level(spdlog::level::trace);
        }
    }

    void LoggerManager::Initialize() {
        if (initialized_) {
            return;
        }

        auto logger_config = Config::Lookup("logs", ankerl::unordered_dense::set<LoggerConfig>(), "Logger configurations");
        logger_config->AddListener(0, [this](const auto& old_configs, const auto& new_configs) -> void {
            OnConfigChange(old_configs, new_configs);
        });

        OnConfigChange({}, logger_config->value());
        initialized_ = true;
    }

    void LoggerManager::OnConfigChange(const ankerl::unordered_dense::set<LoggerConfig>& old_configs,
                                       const ankerl::unordered_dense::set<LoggerConfig>& new_configs)
    {
        StringHeteroHashMap<LoggerConfig> new_configs_map;
        for (const auto& config : new_configs) {
            new_configs_map.try_emplace(config.name, config);
        }

        for (const auto& [name, new_config] : new_configs_map) {
            auto old_it = std::ranges::find_if(old_configs, [&name](const LoggerConfig& old_config) -> bool {
                return old_config.name == name;
            });

            if (old_it == old_configs.end() || !(*old_it == new_config)) {
                ApplyConfig(new_config);
                GLSLD_LOG_INFO(GLSLD_LOG_ROOT(), "Logger [{}] reconfigured.", name);
            }
        }

        for (const auto& old_config : old_configs) {
            if (!new_configs_map.contains(old_config.name)) {
                GLSLD_LOG_INFO(GLSLD_LOG_ROOT(), "Logger [{}] removed.", old_config.name);
                spdlog::drop(old_config.name);
            }
        }
    }

    namespace {
        spdlog::sink_ptr CreateSink(const LogSinkConfig& sink_define) {
            if (sink_define.type == "stdout_color_sink_mt") {
                auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                sink->set_color(spdlog::level::trace, FOREGROUND_BLUE);
                return sink;
            }

            if (sink_define.type == "basic_file_sink_mt") {
                if (sink_define.filename.empty()) {
                    return nullptr;
                }
                return std::make_shared<spdlog::sinks::basic_file_sink_mt>(utils::GetFilePath(sink_define.filename), true);
            }

            return nullptr;
        }
    }

    void LoggerManager::ApplyConfig(const LoggerConfig& config) {
        spdlog::drop(config.name);

        std::vector<spdlog::sink_ptr> sinks;
        for (const auto& sink_define : config.sinks) {
            auto sink = CreateSink(sink_define);
            if (sink != nullptr) {
                sink->set_level(sink_define.level);
                if (!sink_define.pattern.empty()) {
                    sink->set_pattern(sink_define.pattern);
                }

                sinks.push_back(sink);
            }
        }

        std::shared_ptr<spdlog::logger> logger;
        if (sinks.empty()) {
            logger = spdlog::get("null_logger");
            if (logger == nullptr) {
                logger = std::make_shared<spdlog::logger>("null_logger");
                spdlog::register_logger(logger);
            }
        } else {
            logger = std::make_shared<spdlog::logger>(config.name, sinks.begin(), sinks.end());
        }

        logger->set_level(config.level);
        logger->flush_on(config.level);
        if (!config.pattern.empty()) {
            logger->set_pattern(config.pattern);
        }

        spdlog::register_logger(logger);
    }
}

namespace YAML {
    Node convert<glsld::LogSinkConfig>::encode(const glsld::LogSinkConfig& rhs) {
        Node node;
        node["type"] = rhs.type;

        if (!rhs.filename.empty())
            node["filename"] = rhs.filename;
        if (!rhs.pattern.empty())
            node["pattern"] = rhs.pattern;

        // level always encode
        // auto call YAML::convert<spdlog::level::level_enum>::encode
        node["level"] = rhs.level;

        return node;
    }

    bool convert<glsld::LogSinkConfig>::decode(const Node& node, glsld::LogSinkConfig& rhs) {
        if (!node["type"].IsDefined()) {
            return false;
        }

        rhs.type = node["type"].as<std::string>();
        // when decoding, a reasonable default value should be provided for level
        // to avoid it being uninitialized if not defined in YAML
        // spdlog::level::trace is the lowest level and is usually a safe default
        rhs.level = spdlog::level::trace;

        if (node["level"])
            rhs.level    = node["level"].as<spdlog::level::level_enum>();
        if (node["pattern"])
            rhs.pattern  = node["pattern"].as<std::string>();
        if (node["filename"])
            rhs.filename = node["filename"].as<std::string>();

        return true;
    }

    Node convert<glsld::LoggerConfig>::encode(const glsld::LoggerConfig& rhs) {
        Node node;
        node["name"]  = rhs.name;
        node["level"] = rhs.level; // use existed level enum converter

        if (!rhs.pattern.empty()) {
            node["pattern"] = rhs.pattern;
        }

        // foreach sinks vector, call encode function for each sink
        if (!rhs.sinks.empty()) {
            for (const auto& sink_config : rhs.sinks) {
                // node["sinks"] will auto become a Sequence node (vector)
                node["sinks"].push_back(sink_config); // auto call YAML::convert<LogSinkConfig>::encode
            }
        }

        return node;
    }

    bool convert<glsld::LoggerConfig>::decode(const Node& node, glsld::LoggerConfig& rhs) {
        if (!node["name"].IsDefined()) {
            return false;
        }
        rhs.name = node["name"].as<std::string>();

        if (node["level"])
            rhs.level   = node["level"].as<spdlog::level::level_enum>();
        if (node["pattern"])
            rhs.pattern = node["pattern"].as<std::string>();
        if (node["sinks"])
            rhs.sinks   = node["sinks"].as<std::vector<glsld::LogSinkConfig>>();

        return true;
    }
} // namespace YAML
