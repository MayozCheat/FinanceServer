#include "core/HttpServer.h"
#include "core/Logger.h"

void HttpServer::Listen(const std::string& host, int port) {
    // 注意：这里必须是英文双引号 " " ，不能是中文引号 “ ”
    Logger::Instance().Info("[启动] HTTP服务监听: " + host + ":" + std::to_string(port));
    server_.listen(host.c_str(), port);
}
