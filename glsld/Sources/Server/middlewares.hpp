#pragma once

#include "http/context.hpp"

namespace valky::server {
    // @brief 创建一个日志中间件。
    // @details 记录每个请求的详细信息，包括方法、路径、状态码和处理耗时。
    // @return MiddlewareFunc
    http::MiddlewareFunc LoggerMiddleware();

    // @brief 创建一个异常恢复中间件。
    // @details 捕获处理链中未被处理的异常，防止服务器崩溃，并返回一个 500 错误。
    // @return MiddlewareFunc
    http::MiddlewareFunc RecoveryMiddleware();

    // @brief 创建一个 JSON Body 解析中间件。
    // @details 检查请求是否为 application/json，并解析请求体。
    //          如果解析成功，将 nlohmann::json 对象存入 context->user_data["json_body"]。
    //          如果解析失败，直接返回 400 错误。
    // @return MiddlewareFunc
    http::MiddlewareFunc JsonBodyParser();
} // namespace valky::server
