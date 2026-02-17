#pragma once
#include <string>
#include "json.hpp"

using Json = nlohmann::json;

/**
 * @brief 统一接口返回格式（网络字段用英文）
 * ok=true 表示成功
 */
struct ApiResult {
    bool ok{true};
    int code{0};
    std::string msg{"success"};
    Json data = Json::object();

    Json ToJson() const {
        return Json{
            {"ok", ok},
            {"code", code},
            {"msg", msg},
            {"data", data.is_null() ? Json(nullptr) : data}
        };
    }

    static ApiResult Ok(const Json& data = Json::object()) {
        ApiResult r;
        r.ok = true; r.code = 0; r.msg = "success"; r.data = data;
        return r;
    }

    static ApiResult Fail(int code, const std::string& msg) {
        ApiResult r;
        r.ok = false; r.code = code; r.msg = msg; r.data = nullptr;
        return r;
    }
};
