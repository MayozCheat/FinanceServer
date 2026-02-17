#pragma once
#include <string>

/**
 * @brief 文件读取工具（专门给静态网页返回用）
 */
class FileUtil {
public:
    static std::string ReadAllText(const std::string& path);
};
