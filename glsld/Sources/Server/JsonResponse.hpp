#pragma once

#include <nlohmann/json.hpp>
#include <string_view>

namespace glsld {
    void SendResponse(int id, const nlohmann::json& result);
    void SendError(int id, int code, std::string_view message);
    void SendNotification(std::string_view method, const nlohmann::json& params);
}
