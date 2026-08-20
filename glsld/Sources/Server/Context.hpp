#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

namespace glsld {
    using CancellationToken = std::shared_ptr<std::atomic<bool>>;

    struct Context {
        std::string                                method;
        std::optional<nlohmann::json>              request_id;
        std::optional<nlohmann::json>              response;
        std::optional<std::pair<int, std::string>> error;
        nlohmann::json                             params;
        CancellationToken                          cancellation_token{ nullptr };

        bool cancelled() const noexcept {
            return cancellation_token != nullptr && cancellation_token->load(std::memory_order::relaxed);
        }
    };

    using HandlerFunc = std::function<void(Context&)>;
}

#define ABORT_IF_CANCELLED()                                              \
    do {                                                                  \
        if (context.cancelled()) {                                        \
            context.error = std::make_pair(-32800, "Request cancelled."); \
            return {};                                                    \
        }                                                                 \
    } while (false)
