#include "pch.hpp"
#include "JsonResponse.hpp"

#include <cstdio>
#include <mutex>
#include <print>
#include <string_view>

#include "Base/Logger.hpp"

namespace glsld {
    namespace {
        std::mutex send_mutex;

        void SendRaw(std::string_view content) {
            std::lock_guard lock(send_mutex);
            GLSLD_LOG(trace, "Sending response: {}", content);
            std::print("Content-Length: {}\r\n\r\n{}", content.length(), content);
            std::fflush(stdout);
        }

        std::string SerializeJson(const nlohmann::json& value) {
            return value.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
        }
    }

    void SendServerRequest(const nlohmann::json& id, std::string_view method, const nlohmann::json& params) {
        nlohmann::json request{
            { "jsonrpc", "2.0" },
            { "id", id },
            { "method", method }
        };

        if (!params.is_null()) {
            request["params"] = params;
        }

        SendRaw(SerializeJson(request));
    }

    void SendResponse(const nlohmann::json& id, const nlohmann::json& result) {
        nlohmann::json response{
            { "jsonrpc", "2.0" },
            { "id", id },
            { "result", result }
        };

        SendRaw(SerializeJson(response));
    }

    void SendError(const nlohmann::json& id, int code, std::string_view message) {
        nlohmann::json response{
            { "jsonrpc", "2.0" },
            { "id", id },
            { "error", {
                { "code", code },
                { "message", message }
            } }
        };

        SendRaw(SerializeJson(response));
    }

    void SendNotification(std::string_view method, const nlohmann::json& params) {
        nlohmann::json response{
            { "jsonrpc", "2.0" },
            { "method", method },
            { "params", params }
        };

        SendRaw(SerializeJson(response));
    }
}
