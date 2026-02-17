#pragma once
#include <string>
#include "json.hpp"

using Json = nlohmann::json;

/**
 * @brief 配置模块：只负责读取 config/app.json
 */
class Config {
public:
    struct ServerCfg { std::string host="0.0.0.0"; int port=8080; };
    struct MySqlCfg  { std::string host="127.0.0.1"; int port=3306; std::string user="root"; std::string password=""; std::string database="finance_db"; };

    bool Load(const std::string& path);

    const ServerCfg& Server() const { return server_; }
    const MySqlCfg&  MySql() const  { return mysql_; }
    const std::string& WebRoot() const { return webroot_; }

private:
    ServerCfg server_;
    MySqlCfg mysql_;
    std::string webroot_ = "./webroot";
};
