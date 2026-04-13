#pragma once

#include <any>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>
#include "Base/Hash.hpp"

namespace glsld {
    struct Context {
        std::string                                method;
        std::optional<nlohmann::json>              request_id;
        std::optional<nlohmann::json>              response;
        std::optional<std::pair<int, std::string>> error;
        StringHeteroHashMap<std::any>              use_data;
        nlohmann::json                             params;

        std::shared_ptr<std::atomic<bool>> cancelled_token{
            std::make_shared<std::atomic<bool>>(false)
        };

        bool cancelled() const noexcept {
            return cancelled_token != nullptr && cancelled_token->load(std::memory_order::relaxed);
        }
    };

    using HandlerFunc    = std::function<void(Context&)>;
    using MiddlewareFunc = std::function<HandlerFunc(HandlerFunc)>;
}

#define ABORT_IF_CANCELLED()                                              \
    do {                                                                  \
        if (context.cancelled()) {                                        \
            context.error = std::make_pair(-32800, "Request cancelled."); \
            return {};                                                    \
        }                                                                 \
    } while (false)
