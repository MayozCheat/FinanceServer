#pragma once
#include <string>
#include <mutex>
#include <iostream>

/**
 * @brief 日志模块（服务端输出尽量中文，方便你理解）
 * 说明：
 * - 网络传输 JSON 字段我们用英文
 * - 服务端控制台日志用中文
 */
class Logger {
public:
    enum class Level { Info, Warn, Error };

    static Logger& Instance();

    void Info(const std::string& msg);
    void Warn(const std::string& msg);
    void Error(const std::string& msg);

private:
    Logger() = default;
    std::mutex mu_;

    void Log(Level lv, const std::string& msg);
    static std::string Now();
    static const char* LevelText(Level lv);
};
