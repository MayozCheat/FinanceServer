#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mysql/jdbc.h>

/**
 * @brief Db：数据库访问层（基础设施）
 * 负责：
 * - 建立连接
 * - 执行查询（第一版先用 Statement + 返回字符串）
 * 后续扩展：
 * - 连接池
 * - 事务
 * - 预处理语句（防注入）
 */
class Db {
public:
    struct Row { std::vector<std::string> cols; };

    bool Connect(const std::string& host, int port,
                 const std::string& user,
                 const std::string& password,
                 const std::string& database);

    bool Query(const std::string& sqlText,
               std::vector<std::string>& outColNames,
               std::vector<Row>& outRows,
               std::string& outErr);

private:
    std::unique_ptr<sql::Connection> conn_;
};
