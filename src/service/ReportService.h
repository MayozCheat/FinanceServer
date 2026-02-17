#pragma once
#include <string>
#include "core/Result.h"
#include "repo/ReportRepo.h"

/**
 * @brief ReportService：报表业务层
 * 负责：
 * - 参数校验
 * - 未来加入权限校验（谁能看哪个公司）
 * - 调用 Repo 获取数据
 */
class ReportService {
public:
    explicit ReportService(ReportRepo& repo) : repo_(repo) {}

    ApiResult CostBenefit(long long companyId, const std::string& dateFrom, const std::string& dateTo);
    ApiResult ApSummary(long long companyId, const std::string& dateFrom, const std::string& dateTo);

private:
    ReportRepo& repo_;
    bool IsDateFormatOk(const std::string& s) const; // 简单校验 YYYY-MM-DD
};
