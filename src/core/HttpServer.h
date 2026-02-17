#pragma once
#include "httplib.h"
#include <string>

class HttpServer {
public:
    httplib::Server& Raw() { return server_; }
    void Listen(const std::string& host, int port);

private:
    httplib::Server server_;
};
