#include "stdafx.h"
#include "JsonResponse.hpp"

#include <cstdio>
#include <print>
#include <string_view>

#ifdef _DEBUG
#include "Base/Logger.hpp"
#endif

namespace glsld {
    namespace {
        void SendRaw(std::string_view content) {
#ifdef _DEBUG
            VALKY_LOG_DEBUG(VALKY_LOG_ROOT(), "Sending response: {}", content);
#endif
            std::print("Content-Length: {}\r\n\r\n{}", content.length(), content);
            std::fflush(stdout);
        }
    }

    void SendResponse(int id, const nlohmann::json& result) {
        nlohmann::json response{
            { "jsonrpc", "2.0" },
            { "id", id },
            { "result", result }
        };

        SendRaw(response.dump());
    }

    void SendError(int id, int code, std::string_view message) {
        nlohmann::json response{
            { "jsonrpc", "2.0" },
            { "id", id },
            { "error", {
                { "code", code },
                { "message", message }
            } }
        };

        SendRaw(response.dump());
    }

    void SendNotification(std::string_view method, const nlohmann::json& params) {
        nlohmann::json response{
            { "jsonrpc", "2.0" },
            { "method", method },
            { "params", params }
        };

        SendRaw(response.dump());
    }
}
