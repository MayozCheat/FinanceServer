#pragma once
#include <string>
#include "core/Result.h"
#include "db/Db.h"

/**
 * @brief ReportRepo：报表仓储（SQL集中在这里）
 * 负责：执行 SQL + 映射为 JSON
 * 不负责：参数校验/权限校验（这些属于 service）
 */
class ReportRepo {
public:
    explicit ReportRepo(Db& db) : db_(db) {}

    ApiResult QueryCostBenefit(long long companyId, const std::string& dateFrom, const std::string& dateTo);
    ApiResult QueryApSummary(long long companyId, const std::string& dateFrom, const std::string& dateTo);

private:
    Db& db_;
};
