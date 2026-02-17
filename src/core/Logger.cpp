#include "core/Logger.h"
#include <ctime>
#include <cstdio>

Logger& Logger::Instance() {
    static Logger ins;
    return ins;
}

void Logger::Info(const std::string& msg)  { Log(Level::Info, msg); }
void Logger::Warn(const std::string& msg)  { Log(Level::Warn, msg); }
void Logger::Error(const std::string& msg) { Log(Level::Error, msg); }

void Logger::Log(Level lv, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mu_);
    std::cout << "[" << Now() << "][" << LevelText(lv) << "] " << msg << std::endl;
}

std::string Logger::Now() {
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
        tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return buf;
}

const char* Logger::LevelText(Level lv) {
    switch (lv) {
    case Level::Info:  return "信息";
    case Level::Warn:  return "警告";
    case Level::Error: return "错误";
    default: return "未知";
    }
}
