#include "service/ReportService.h"
#include "core/Logger.h"
#include <cctype>

bool ReportService::IsDateFormatOk(const std::string& s) const {
    if (s.size() != 10) return false;
    for (size_t i = 0; i < s.size(); i++) {
        if (i==4 || i==7) { if (s[i] != '-') return false; }
        else { if (!std::isdigit((unsigned char)s[i])) return false; }
    }
    return true;
}

ApiResult ReportService::CostBenefit(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    if (companyId <= 0) return ApiResult::Fail(10001, "invalid_company_id");
    if (!IsDateFormatOk(dateFrom) || !IsDateFormatOk(dateTo)) return ApiResult::Fail(10002, "invalid_date");

    Logger::Instance().Info("【报表】成本效益查询：company_id=" + std::to_string(companyId));
    return repo_.QueryCostBenefit(companyId, dateFrom, dateTo);
}

ApiResult ReportService::ApSummary(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    if (companyId <= 0) return ApiResult::Fail(10001, "invalid_company_id");
    if (!IsDateFormatOk(dateFrom) || !IsDateFormatOk(dateTo)) return ApiResult::Fail(10002, "invalid_date");

    Logger::Instance().Info("【报表】应付汇总查询：company_id=" + std::to_string(companyId));
    return repo_.QueryApSummary(companyId, dateFrom, dateTo);
}
