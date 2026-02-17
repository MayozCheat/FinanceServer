#pragma once
#include "core/HttpServer.h"
#include "service/ReportService.h"

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
    explicit ApiController(ReportService& reportService) : reportService_(reportService) {}
    void Register(HttpServer& http);

private:
    ReportService& reportService_;
};
