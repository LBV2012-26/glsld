#include <utility>
#include "Base/Hash.hpp"

namespace glsld {
    inline bool LogSinkConfig::operator==(const LogSinkConfig& other) const {
        return type     == other.type     && pattern == other.pattern &&
               filename == other.filename && level   == other.level;
    }

    inline std::shared_ptr<spdlog::logger> LoggerManager::GetLogger(std::string_view name) {
        auto logger = spdlog::get(std::string(name));
        if (logger == nullptr) {
            return GetRoot();
        }

        return logger;
    }

    inline std::shared_ptr<spdlog::logger> LoggerManager::GetRoot() {
        return spdlog::get("root");
    }

    inline LoggerManager& LoggerManager::GetInstance() {
        static LoggerManager instance;
        return instance;
    }
}

namespace std {
    inline size_t hash<glsld::LogSinkConfig>::operator()(const glsld::LogSinkConfig& config) const {
        size_t seed = 0;
        glsld::HashCombine(seed, config.type);
        glsld::HashCombine(seed, config.pattern);
        glsld::HashCombine(seed, config.filename);
        glsld::HashCombine(seed, std::to_underlying(config.level));
        return seed;
    }

    inline size_t hash<glsld::LoggerConfig>::operator()(const glsld::LoggerConfig& config) const {
        size_t seed = 0;
        glsld::HashCombine(seed, config.name);
        glsld::HashCombine(seed, config.pattern);
        glsld::HashCombine(seed, std::to_underlying(config.level));

        for (const auto& sink : config.sinks) {
            glsld::HashCombine(seed, sink);
        }

        return seed;
    }
} // namespace std;

namespace YAML {
    inline Node convert<spdlog::level::level_enum>::encode(const spdlog::level::level_enum& rhs) {
        return Node(spdlog::level::to_short_c_str(rhs));
    }

    inline bool convert<spdlog::level::level_enum>::decode(const Node& node, spdlog::level::level_enum& rhs) {
        if (!node.IsScalar()) {
            return false;
        }

        rhs = spdlog::level::from_str(node.as<std::string>());
        return true;
    }
} // namespace YAML
