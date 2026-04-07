#pragma once

#include <any>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
#include "Base/Hash.hpp"

namespace glsld {
    struct Context {
        std::string                                method;
        std::optional<nlohmann::json>              response;
        std::optional<std::pair<int, std::string>> error;
        StringHeteroHashTable<std::any>            use_data;
        nlohmann::json                             params;
    };

    using HandlerFunc    = std::function<void(Context&)>;
    using MiddlewareFunc = std::function<HandlerFunc(HandlerFunc)>;
}
