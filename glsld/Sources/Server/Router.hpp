#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>
#include "Server/Context.hpp"
#include "Utils/Utils.hpp"

namespace glsld {
    using RequestHandler      = std::function<nlohmann::json(Context&)>;
    using NotificationHandler = std::function<void(Context&)>;

    class Router {
    public:
        void Use(MiddlewareFunc middleware);
        void RegisterRequest(std::string_view method, RequestHandler handler);
        void RegisterNotification(std::string_view method, NotificationHandler handler);
        void Dispatch(Context& context, bool is_request);

    private:
        HandlerFunc BuildPipeline(HandlerFunc handler);

        std::vector<MiddlewareFunc>                            middlewares_;
        utils::StringHeteroHashTable<std::string, HandlerFunc> request_routes_;
        utils::StringHeteroHashTable<std::string, HandlerFunc> notification_routes_;
        HandlerFunc                                            not_found_handler_;
    };
}
