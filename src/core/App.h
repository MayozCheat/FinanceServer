#pragma once
#include <memory>
#include "core/Config.h"
#include "core/HttpServer.h"
#include "db/Db.h"
#include "repo/ReportRepo.h"
#include "service/ReportService.h"
#include "service/AuthService.h"
#include "web/ApiController.h"
#include "web/AuthController.h"
#include "web/StaticController.h"

/**
 * @brief App：应用总装配（依赖注入中心）
 * 负责：
 * 1) 读取配置
 * 2) 初始化数据库
 * 3) 构建 Repo/Service/Controller
 * 4) 注册路由
 * 5) 启动服务
 */
class App {
public:
    bool Init(const std::string& configPath);
    void Run();

private:
    Config cfg_;
    HttpServer http_;
    Db db_;

    std::unique_ptr<ReportRepo> reportRepo_;
    std::unique_ptr<ReportService> reportService_;
    std::unique_ptr<AuthService> authService_;
    std::unique_ptr<ApiController> apiController_;
    std::unique_ptr<AuthController> authController_;
    std::unique_ptr<StaticController> staticController_;
};
