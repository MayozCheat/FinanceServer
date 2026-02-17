#include "core/App.h"
#include "core/Logger.h"

bool App::Init(const std::string& configPath) {
    Logger::Instance().Info("【启动】读取配置：" + configPath);
    if (!cfg_.Load(configPath)) {
        Logger::Instance().Error("【启动】读取配置失败：请检查 config/app.json");
        return false;
    }

    const auto& m = cfg_.MySql();
    if (!db_.Connect(m.host, m.port, m.user, m.password, m.database)) {
        Logger::Instance().Error("【启动】数据库连接失败，请检查账号密码/库名");
        return false;
    }

    reportRepo_ = std::make_unique<ReportRepo>(db_);
    financeRepo_ = std::make_unique<FinanceRepo>(db_);
    reportService_ = std::make_unique<ReportService>(*reportRepo_);
    financeService_ = std::make_unique<FinanceService>(*financeRepo_);
    authService_ = std::make_unique<AuthService>();
    apiController_ = std::make_unique<ApiController>(*reportService_, *financeService_, *authService_);
    authController_ = std::make_unique<AuthController>(*authService_);
    staticController_ = std::make_unique<StaticController>(cfg_.WebRoot());

    apiController_->Register(http_);
    authController_->Register(http_);
    staticController_->Register(http_);

    Logger::Instance().Info("【启动】初始化完成");
    return true;
}

void App::Run() {
    http_.Listen(cfg_.Server().host, cfg_.Server().port);
}
