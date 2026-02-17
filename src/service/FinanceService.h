#pragma once
#include <string>
#include "core/Result.h"
#include "repo/FinanceRepo.h"

class FinanceService {
public:
    explicit FinanceService(FinanceRepo& repo) : repo_(repo) {}

    ApiResult ListCompanies();
    ApiResult ListProjects(long long companyId);

    ApiResult CreateCompany(long long companyId, const std::string& name);
    ApiResult CreateProject(long long projectId, long long companyId, const std::string& name);

    ApiResult CreateCostBenefitRecord(long long companyId, long long projectId, const std::string& month,
                                      double outputValue, double tax, double materialCost,
                                      double machineCost, double machineDeprCost, double laborMgmtCost,
                                      double laborProjectCost, double otherCost, double financeFee,
                                      double nonprodIncome, double nonprodExpense, double incomeTax,
                                      double assessProfit, const std::string& remark);

    ApiResult CreateApAccrualRecord(long long companyId, long long projectId, const std::string& vendorName,
                                    const std::string& bizType, double amount, const std::string& bizDate);

    ApiResult CreateApPaymentRecord(long long companyId, long long projectId, const std::string& vendorName,
                                    const std::string& bizType, double amount, const std::string& payDate);

    ApiResult ListApAccrual(long long companyId, const std::string& dateFrom, const std::string& dateTo);
    ApiResult ListApPayment(long long companyId, const std::string& dateFrom, const std::string& dateTo);

private:
    static bool IsDateFormatOk(const std::string& s);

    FinanceRepo& repo_;
};
