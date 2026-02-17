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
    auto requireLogin = [this](const httplib::Request& req, long long& userId, bool& isAdmin, httplib::Response& res) {
        std::string token;
        if (!ExtractBearerToken(req, token)) {
            ReplyJson(res, ApiResult::Fail(30005, "missing_or_invalid_token"));
            return false;
        }

        if (!authService_.ValidateToken(token, userId, isAdmin)) {
            ReplyJson(res, ApiResult::Fail(30006, "invalid_token"));
            return false;
        }

        return true;
    };

    s.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("ok", "text/plain; charset=utf-8");
    });

    s.Get("/api/companies", [this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;
        (void)userId;
        (void)isAdmin;

        ReplyJson(res, financeService_.ListCompanies());
    });

    s.Get("/api/projects", [this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        long long companyId = 0;
        if (req.has_param("company_id")) {
            try {
                companyId = std::stoll(req.get_param_value("company_id"));
            } catch (...) {
                ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
                return;
            }

            if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
                ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
                return;
            }
        }

        ReplyJson(res, financeService_.ListProjects(companyId));
    });

    s.Post("/api/admin/companies", httplib::Server::Handler([this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;
        if (!isAdmin) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        ReplyJson(res, financeService_.CreateCompany(body.value("id", 0LL), body.value("name", "")));
    }));

    s.Post("/api/admin/projects", httplib::Server::Handler([this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;
        if (!isAdmin) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        ReplyJson(res, financeService_.CreateProject(body.value("id", 0LL), body.value("companyId", 0LL), body.value("name", "")));
    }));

    s.Get("/api/reports/cost_benefit", [this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        long long companyId = 0;
        try {
            companyId = std::stoll(req.get_param_value("company_id"));
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
            return;
        }

        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        std::string dateFrom = req.get_param_value("date_from");
        std::string dateTo = req.get_param_value("date_to");
        ReplyJson(res, reportService_.CostBenefit(companyId, dateFrom, dateTo));
    });

    s.Get("/api/reports/ap_summary", [this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        long long companyId = 0;
        try {
            companyId = std::stoll(req.get_param_value("company_id"));
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
            return;
        }

        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        std::string dateFrom = req.get_param_value("date_from");
        std::string dateTo = req.get_param_value("date_to");
        ReplyJson(res, reportService_.ApSummary(companyId, dateFrom, dateTo));
    });

    s.Post("/api/finance/cost_benefit", httplib::Server::Handler([this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        long long companyId = body.value("companyId", 0LL);
        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        ReplyJson(res, financeService_.CreateCostBenefitRecord(
            companyId,
            body.value("projectId", 0LL),
            body.value("month", ""),
            body.value("outputValue", 0.0),
            body.value("tax", 0.0),
            body.value("materialCost", 0.0),
            body.value("machineCost", 0.0),
            body.value("machineDeprCost", 0.0),
            body.value("laborMgmtCost", 0.0),
            body.value("laborProjectCost", 0.0),
            body.value("otherCost", 0.0),
            body.value("financeFee", 0.0),
            body.value("nonprodIncome", 0.0),
            body.value("nonprodExpense", 0.0),
            body.value("incomeTax", 0.0),
            body.value("assessProfit", 0.0),
            body.value("remark", "")
        ));
    }));

    s.Post("/api/finance/ap_accrual", httplib::Server::Handler([this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        long long companyId = body.value("companyId", 0LL);
        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        ReplyJson(res, financeService_.CreateApAccrualRecord(
            companyId,
            body.value("projectId", 0LL),
            body.value("vendorName", ""),
            body.value("bizType", ""),
            body.value("amount", 0.0),
            body.value("bizDate", "")
        ));
    }));

    s.Post("/api/finance/ap_payment", httplib::Server::Handler([this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        Json body;
        try {
            body = Json::parse(req.body);
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(30000, "invalid_json"));
            return;
        }

        long long companyId = body.value("companyId", 0LL);
        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        ReplyJson(res, financeService_.CreateApPaymentRecord(
            companyId,
            body.value("projectId", 0LL),
            body.value("vendorName", ""),
            body.value("bizType", ""),
            body.value("amount", 0.0),
            body.value("payDate", "")
        ));
    }));

    s.Get("/api/finance/ap_accrual", [this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        long long companyId = 0;
        try {
            companyId = std::stoll(req.get_param_value("company_id"));
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
            return;
        }

        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        ReplyJson(res, financeService_.ListApAccrual(companyId, req.get_param_value("date_from"), req.get_param_value("date_to")));
    });

    s.Get("/api/finance/ap_payment", [this, &requireLogin](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        long long userId = 0;
        bool isAdmin = false;
        if (!requireLogin(req, userId, isAdmin, res)) return;

        long long companyId = 0;
        try {
            companyId = std::stoll(req.get_param_value("company_id"));
        } catch (...) {
            ReplyJson(res, ApiResult::Fail(10001, "invalid_company_id"));
            return;
        }

        if (!isAdmin && !authService_.CanAccessCompany(userId, companyId)) {
            ReplyJson(res, ApiResult::Fail(30002, "forbidden"));
            return;
        }

        ReplyJson(res, financeService_.ListApPayment(companyId, req.get_param_value("date_from"), req.get_param_value("date_to")));
    });
}
