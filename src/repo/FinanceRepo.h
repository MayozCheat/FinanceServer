#pragma once
#include <string>
#include "core/Result.h"
#include "db/Db.h"

class FinanceRepo {
public:
    explicit FinanceRepo(Db& db) : db_(db) {}

    ApiResult ListCompanies();
    ApiResult ListProjects(long long companyId);

    ApiResult CreateCompany(long long companyId, const std::string& name);
    ApiResult CreateProject(long long projectId, long long companyId, const std::string& name);

    ApiResult UpsertCostBenefit(long long companyId, long long projectId, const std::string& month,
                                double outputValue, double tax, double materialCost,
                                double machineCost, double machineDeprCost, double laborMgmtCost,
                                double laborProjectCost, double otherCost, double financeFee,
                                double nonprodIncome, double nonprodExpense, double incomeTax,
                                double assessProfit, const std::string& remark);

    ApiResult CreateApAccrual(long long companyId, long long projectId, const std::string& vendorName,
                              const std::string& bizType, double amount, const std::string& bizDate);

    ApiResult CreateApPayment(long long companyId, long long projectId, const std::string& vendorName,
                              const std::string& bizType, double amount, const std::string& payDate);

    ApiResult ListApAccrual(long long companyId, const std::string& dateFrom, const std::string& dateTo);
    ApiResult ListApPayment(long long companyId, const std::string& dateFrom, const std::string& dateTo);

private:
    static std::string EscapeSql(const std::string& s);
    static bool IsTableMissingError(const std::string& err);

    Db& db_;
};
