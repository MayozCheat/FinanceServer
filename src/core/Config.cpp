#include <iostream>  // 确保包含了头文件
#include <fstream>    // 确保包含了文件流头文件
#include "core/Config.h"
#include "json.hpp"   // 如果你使用了 nlohmann/json 解析库，记得包括它

void SetWorkingDirectory() {
    // 设置当前工作目录为 D:/FinanceServer/build/Release/
    std::filesystem::current_path("D:/FinanceServer/build/Release");
}

bool Config::Load(const std::string& path) {

     SetWorkingDirectory();  // 设置工作目录
    // 输出当前工作目录
    std::cout << "当前工作目录：" << std::filesystem::current_path() << std::endl;

    std::cout << "正在读取配置文件路径：" << path << std::endl;  // 打印路径

    // 检查是否能打开配置文件
    std::ifstream ifs(path);  // 这里是文件流，确保没有遗漏分号
    if (!ifs.is_open()) {
        std::cerr << "无法打开文件: " << path << std::endl; // 打印错误信息
        return false;
    }

    // 如果文件成功打开，读取内容
    nlohmann::json j;
    ifs >> j;  // 读取 JSON 文件内容

    std::cout << "配置文件内容：" << j.dump(4) << std::endl;  // 打印 JSON 内容（格式化输出）

    // 加载配置文件内容
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
