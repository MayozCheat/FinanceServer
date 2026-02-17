#pragma once
#include <string>
#include "core/HttpServer.h"

/**
 * @brief StaticController：静态页面返回
 * 负责：
 * - 把 webroot/app/index.html 返回给浏览器
 * 后续：
 * - 你需要多少页面都可以放 webroot 下，通过这里映射
 */
class StaticController {
public:
    explicit StaticController(std::string webroot) : webroot_(std::move(webroot)) {}
    void Register(HttpServer& http);

private:
    std::string webroot_;
};
