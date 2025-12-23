#include "pch.hpp"
#include "middlewares.hpp"

#include <chrono>
#include <exception>
#include <utility>

#include <coro/coro.hpp>
#include <nlohmann/json.hpp>
#include "base/logger.hpp"
#include "http/http.hpp"
#include "server/json_response.hpp"

namespace valky::server {
    http::MiddlewareFunc LoggerMiddleware() {
        return [](http::HandlerFunc next) -> http::HandlerFunc {
            return [next](http::HttpContext& context) -> coro::task<void> {
                auto start    = std::chrono::high_resolution_clock::now();
                co_await next(context);
                auto end      = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                const auto& request    = context.request;
                const auto& response   = context.response;
                auto        user_agent = request.FindHeader("user-agent").value_or("-");

                VALKY_LOG_INFO(VALKY_LOG_ROOT(), "{} {} {} \"{}\" {}us",
                    http::HttpMethodToString(request.method()),
                    request.path(),
                    std::to_underlying(response.status()),
                    user_agent,
                    duration
                );
            };
        };
    }

    http::MiddlewareFunc RecoveryMiddleware() {
        return [](http::HandlerFunc next) -> http::HandlerFunc {
            return [next](http::HttpContext& context) -> coro::task<void> {
                try {
                    co_await next(context);
                } catch (const std::exception& e) {
                    VALKY_LOG_ERROR(VALKY_LOG_ROOT(), "Panic recovered in handler for path '{}': {}",
                                    context.request.path(), e.what());

                    if (std::to_underlying(context.response.status()) < std::to_underlying(http::HttpStatus::kBadRequest)) {
                        context.response.InsertHeader("content-type", "appliaction/json");
                        context.response.set_status(http::HttpStatus::kInternalServerError);
                        context.response.set_body(R"({"error": "Internal Server Error"})");
                    }
                } catch (...) {
                    VALKY_LOG_ERROR(VALKY_LOG_ROOT(), "Unknown panic recovered in handler for path '{}'",
                                    context.request.path());
                    if (std::to_underlying(context.response.status()) < std::to_underlying(http::HttpStatus::kBadRequest)) {
                        context.response.InsertHeader("content-type", "appliaction/json");
                        context.response.set_status(http::HttpStatus::kInternalServerError);
                        context.response.set_body(R"({"error": "Internal Server Error"})");
                    }
                }
            };
        };
    }

    http::MiddlewareFunc JsonBodyParser() {
        return [](http::HandlerFunc next) -> http::HandlerFunc {
            return [next](http::HttpContext& context) -> coro::task<void> {
                auto http_method = context.request.method();
                if (http_method == http::HttpMethod::kPost ||
                    http_method == http::HttpMethod::kPut  ||
                    http_method == http::HttpMethod::kPatch)
                {
                    auto content_type = context.request.FindHeader("content-type");
                    if (content_type.has_value() && content_type->find("application/json") != std::string_view::npos) {
                        if (context.request.body().empty()) {
                            JsonErrorResponse(context, http::HttpStatus::kBadRequest, "JSON body is empty.");
                            co_return;
                        }

                        try {
                            context.user_data["json_body"] = nlohmann::json::parse(context.request.body());
                        } catch (const nlohmann::json::parse_error& e) {
                            JsonErrorResponse(context, http::HttpStatus::kBadRequest, "Invalid JSON format.");
                            co_return;
                        }
                    }
                }

                co_await next(context);
            };
        };
    }
} // namespace valky::server
