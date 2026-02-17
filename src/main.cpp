#include "core/App.h"
#include "core/Logger.h"

void SetConsoleUtf8() {
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8 编码
    SetConsoleCP(CP_UTF8);        // 设置控制台输入为 UTF-8 编码
}

int main() {
    SetConsoleUtf8();  // 设置 UTF-8 编码
    App app;

    const std::string cfgPath = "config/app.json";

    if (!app.Init(cfgPath)) {
        Logger::Instance().Error("[退出] 初始化失败，程序结束");
        return 1;
    }

    Logger::Instance().Info("[提示] 浏览器打开: http://127.0.0.1:8080/app/");
    Logger::Instance().Info("[提示] 健康检查: http://127.0.0.1:8080/health");

    app.Run();
    return 0;
}
