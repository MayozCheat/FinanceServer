#pragma once
#include "core/HttpServer.h"
#include "service/ReportService.h"
#include "service/AuthService.h"

/**
 * @brief ApiController：HTTP接口层
 * 负责：
 * - 路由注册
 * - 参数解析
 * - 调用 Service
 * - 返回统一 JSON（英文字段）
 */
class ApiController {
public:
    ApiController(ReportService& reportService, AuthService& authService)
        : reportService_(reportService), authService_(authService) {}
    void Register(HttpServer& http);

private:
    ReportService& reportService_;
    AuthService& authService_;
};
