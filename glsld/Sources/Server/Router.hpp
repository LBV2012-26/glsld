#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>
#include "Server/Context.hpp"
#include "Base/Hash.hpp"

namespace glsld {
    using RequestHandler      = std::function<nlohmann::json(Context&)>;
    using NotificationHandler = std::function<void(Context&)>;

    class Router {
    public:
        void RegisterRequest(std::string_view method, RequestHandler handler);
        void RegisterNotification(std::string_view method, NotificationHandler handler);
        void Dispatch(Context& context, bool is_request);

    private:
        StringHeteroHashMap<HandlerFunc> request_routes_;
        StringHeteroHashMap<HandlerFunc> notification_routes_;
        HandlerFunc                      not_found_handler_;
    };
}
