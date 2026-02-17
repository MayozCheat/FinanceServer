#include "web/ApiController.h"
#include "core/Result.h"

static void ReplyJson(httplib::Response& res, const ApiResult& r) {
    auto j = r.ToJson();
    res.set_header("Content-Type", "application/json; charset=utf-8");
    res.set_content(j.dump(), "application/json; charset=utf-8");
}

static bool ExtractBearerToken(const httplib::Request& req, std::string& token) {
    std::string auth = req.get_header_value("Authorization");
    const std::string prefix = "Bearer ";
    if (auth.rfind(prefix, 0) != 0) return false;
    token = auth.substr(prefix.size());
    return !token.empty();
}

void ApiController::Register(HttpServer& http) {
    auto& s = http.Raw();

    s.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("ok", "text/plain; charset=utf-8");
    });

    s.Get("/api/reports/cost_benefit", [this](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        std::string token;
        if (!ExtractBearerToken(req, token)) {
            ReplyJson(res, ApiResult::Fail(30005, "missing_or_invalid_token"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!authService_.ValidateToken(token, userId, isAdmin)) {
            ReplyJson(res, ApiResult::Fail(30006, "invalid_token"));
            return;
        }

        long long companyId = 0;
        try {
            companyId = std::stoll(req.get_param_value("company_id"));
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
            return;
        }

        if (!authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        std::string dateFrom = req.get_param_value("date_from");
        std::string dateTo = req.get_param_value("date_to");
        ReplyJson(res, reportService_.CostBenefit(companyId, dateFrom, dateTo));
    });

    s.Get("/api/reports/ap_summary", [this](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        std::string token;
        if (!ExtractBearerToken(req, token)) {
            ReplyJson(res, ApiResult::Fail(30005, "missing_or_invalid_token"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!authService_.ValidateToken(token, userId, isAdmin)) {
            ReplyJson(res, ApiResult::Fail(30006, "invalid_token"));
            return;
        }

        long long companyId = 0;
        try {
            companyId = std::stoll(req.get_param_value("company_id"));
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
            return;
        }

        if (!authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        std::string dateFrom = req.get_param_value("date_from");
        std::string dateTo = req.get_param_value("date_to");
        ReplyJson(res, reportService_.ApSummary(companyId, dateFrom, dateTo));
    });
}
