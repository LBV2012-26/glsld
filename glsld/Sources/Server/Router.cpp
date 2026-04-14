#include "stdafx.h"
#include "Router.hpp"

#include <exception>
#include <format>
#include <string>
#include <utility>

#include "Base/Logger.hpp"

namespace glsld {
    namespace {
        void DefaultNotFoundHandler(Context& context) {
            context.error = std::make_pair(-32601, std::format("Method Not Found: {}", context.method));
            return;
        }
    }

    void Router::Use(MiddlewareFunc middleware) {
        middlewares_.push_back(std::move(middleware));
    }

    void Router::RegisterRequest(std::string_view method, RequestHandler handler) {
        HandlerFunc adapter = [handler](Context& context) -> void {
            try {
                nlohmann::json response = handler(context);
                context.response = std::move(response);
            } catch (const std::exception& e) {
                context.error = std::make_pair(-32603, std::format("Internal Error: {}", e.what()));
            }
        };

        request_routes_[std::string(method)] = BuildPipeline(std::move(adapter));
    }

    void Router::RegisterNotification(std::string_view method, NotificationHandler handler) {
        HandlerFunc adapter = [handler](Context& context) -> void {
            try {
                handler(context);
            } catch (const std::exception& e) {
                GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Notification handler error: {}", e.what());
            }
        };

        notification_routes_[std::string(method)] = BuildPipeline(std::move(adapter));
    }

    void Router::Dispatch(Context& context, bool is_request) {
        if (is_request) {
            auto it = request_routes_.find(context.method);
            if (it != request_routes_.end()) {
                it->second(context);
            } else {
                context.error = std::make_pair(-32601, std::format("Method Not Found: {}", context.method));
            }
        } else { // notification
            auto it = notification_routes_.find(context.method);
            if (it != notification_routes_.end()) {
                it->second(context);
            } else {
                GLSLD_LOG_ERROR(GLSLD_LOG_ROOT(), "Unhandled notification: {}", context.method);
            }
        }
    }

    HandlerFunc Router::BuildPipeline(HandlerFunc handler) {
        HandlerFunc chain = std::move(handler);

        for (auto it = middlewares_.rbegin(); it != middlewares_.rend(); ++it) {
            chain = (*it)(chain);
        }
        return chain;
    }
}
