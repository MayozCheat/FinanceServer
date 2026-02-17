#include "web/ApiController.h"
#include "core/Result.h"

static void ReplyJson(httplib::Response& res, const ApiResult& r) {
    auto j = r.ToJson();  // 转换为 JSON 格式
    res.set_header("Content-Type", "application/json; charset=utf-8");
    res.set_content(j.dump(), "application/json; charset=utf-8");  // 设置响应内容和类型
}

void ApiController::Register(HttpServer& http) {
    auto& s = http.Raw();

    // 健康检查接口
    s.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("ok", "text/plain; charset=utf-8");
        });

    // 通过 GET 请求查询成本效益报表
    s.Get("/api/reports/cost_benefit", [this](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        long long companyId = std::stoll(req.get_param_value("company_id"));
        std::string dateFrom = req.get_param_value("date_from");
        std::string dateTo = req.get_param_value("date_to");

        // 查询数据并返回
        ReplyJson(res, reportService_.CostBenefit(companyId, dateFrom, dateTo));
        });

    // 通过 GET 请求查询应付汇总报表
    s.Get("/api/reports/ap_summary", [this](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("company_id") || !req.has_param("date_from") || !req.has_param("date_to")) {
            ReplyJson(res, ApiResult::Fail(10000, "missing_params"));
            return;
        }

        long long companyId = std::stoll(req.get_param_value("company_id"));
        std::string dateFrom = req.get_param_value("date_from");
        std::string dateTo = req.get_param_value("date_to");

        // 查询数据并返回
        ReplyJson(res, reportService_.ApSummary(companyId, dateFrom, dateTo));
        });


}
