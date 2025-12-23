#pragma once

#include <any>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

namespace glsld {
    struct Context {
        std::string                                method;
        std::optional<nlohmann::json>              response;
        std::optional<std::pair<int, std::string>> error;
        std::unordered_map<std::string, std::any>  use_data;
        nlohmann::json                             params;
    };

    using HandlerFunc    = std::function<void(Context&)>;
    using MiddlewareFunc = std::function<HandlerFunc(HandlerFunc)>;
}
