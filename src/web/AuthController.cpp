#include "web/AuthController.h"
#include "core/Result.h"

static void ReplyJsonAuth(httplib::Response& res, const ApiResult& r) {
    auto j = r.ToJson();
    res.set_content(j.dump(), "application/json; charset=utf-8");
}

static bool ExtractBearerToken(const httplib::Request& req, std::string& token) {
    std::string auth = req.get_header_value("Authorization");
    const std::string prefix = "Bearer ";
    if (auth.rfind(prefix, 0) != 0) return false;
    token = auth.substr(prefix.size());
    return !token.empty();
}

void AuthController::Register(HttpServer& http) {
    auto& s = http.Raw();

    s.Post("/api/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJsonAuth(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        std::string username = body.value("username", "");
        std::string password = body.value("password", "");
        if (username.empty() || password.empty()) {
            ReplyJsonAuth(res, ApiResult::Fail(30003, "invalid_params"));
            return;
        }

        ReplyJsonAuth(res, authService_.Login(username, password));
    });

    s.Get("/api/admin/permissions", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token;
        if (!ExtractBearerToken(req, token)) {
            ReplyJsonAuth(res, ApiResult::Fail(30005, "missing_or_invalid_token"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!authService_.ValidateToken(token, userId, isAdmin)) {
            ReplyJsonAuth(res, ApiResult::Fail(30006, "invalid_token"));
            return;
        }

        ReplyJsonAuth(res, authService_.ListPermissions(userId, isAdmin));
    });

    s.Post("/api/admin/permissions/company", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token;
        if (!ExtractBearerToken(req, token)) {
            ReplyJsonAuth(res, ApiResult::Fail(30005, "missing_or_invalid_token"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!authService_.ValidateToken(token, userId, isAdmin)) {
            ReplyJsonAuth(res, ApiResult::Fail(30006, "invalid_token"));
            return;
        }

        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJsonAuth(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        long long targetUserId = body.value("targetUserId", 0LL);
        long long companyId = body.value("companyId", 0LL);
        bool allow = body.value("allow", false);

        ReplyJsonAuth(res, authService_.SetCompanyAccess(userId, isAdmin, targetUserId, companyId, allow));
    });
}
