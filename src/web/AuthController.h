#pragma once
#include "core/HttpServer.h"
#include "service/AuthService.h"

class AuthController {
public:
    explicit AuthController(AuthService& authService) : authService_(authService) {}
    void Register(HttpServer& http);

private:
    AuthService& authService_;
};
