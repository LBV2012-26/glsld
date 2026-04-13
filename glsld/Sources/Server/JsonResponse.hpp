#pragma once

#include <string_view>
#include <nlohmann/json.hpp>

namespace glsld {
    void SendResponse(const nlohmann::json& id, const nlohmann::json& result);
    void SendError(const nlohmann::json& id, int code, std::string_view message);
    void SendNotification(std::string_view method, const nlohmann::json& params);
}
