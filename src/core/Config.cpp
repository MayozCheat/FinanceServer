#include "core/Config.h"
#include "core/Logger.h"
#include <filesystem>
#include <fstream>

namespace {
void SetWorkingDirectory() {
    // Windows 部署时保持与现有发布目录一致
    std::filesystem::current_path("D:/FinanceServer/build/Release");
}
}

bool Config::Load(const std::string& path) {
    SetWorkingDirectory();

    Logger::Instance().Info("[配置] 当前工作目录: " + std::filesystem::current_path().string());
    Logger::Instance().Info("[配置] 读取配置文件: " + path);

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        Logger::Instance().Error("[配置] 无法打开配置文件: " + path);
        return false;
    }

    Json j;
    ifs >> j;

    if (j.contains("server")) {
        server_.host = j["server"].value("host", server_.host);
        server_.port = j["server"].value("port", server_.port);
    }

    if (j.contains("mysql")) {
        mysql_.host = j["mysql"].value("host", mysql_.host);
        mysql_.port = j["mysql"].value("port", mysql_.port);
        mysql_.user = j["mysql"].value("user", mysql_.user);
        mysql_.password = j["mysql"].value("password", mysql_.password);
        mysql_.database = j["mysql"].value("database", mysql_.database);
    }

    webroot_ = j.value("webroot", webroot_);
    return true;
}
