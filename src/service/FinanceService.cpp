#include "service/FinanceService.h"
#include <cctype>

bool FinanceService::IsDateFormatOk(const std::string& s) {
    if (s.size() != 10) return false;
    for (size_t i = 0; i < s.size(); i++) {
        if (i == 4 || i == 7) {
            if (s[i] != '-') return false;
        } else {
            if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        }
    }
    return true;
}

ApiResult FinanceService::ListCompanies() {
    return repo_.ListCompanies();
}

ApiResult FinanceService::ListProjects(long long companyId) {
    if (companyId < 0) return ApiResult::Fail(10001, "invalid_company_id");
    return repo_.ListProjects(companyId);
}

ApiResult FinanceService::CreateCompany(long long companyId, const std::string& name) {
    if (companyId <= 0 || name.empty()) return ApiResult::Fail(30003, "invalid_params");
    return repo_.CreateCompany(companyId, name);
}

ApiResult FinanceService::CreateProject(long long projectId, long long companyId, const std::string& name) {
    if (projectId <= 0 || companyId <= 0 || name.empty()) return ApiResult::Fail(30003, "invalid_params");
    return repo_.CreateProject(projectId, companyId, name);
}

ApiResult FinanceService::CreateCostBenefitRecord(long long companyId, long long projectId, const std::string& month,
                                                  double outputValue, double tax, double materialCost,
                                                  double machineCost, double machineDeprCost, double laborMgmtCost,
                                                  double laborProjectCost, double otherCost, double financeFee,
                                                  double nonprodIncome, double nonprodExpense, double incomeTax,
                                                  double assessProfit, const std::string& remark) {
    if (companyId <= 0 || projectId <= 0 || !IsDateFormatOk(month)) return ApiResult::Fail(30003, "invalid_params");
    return repo_.UpsertCostBenefit(companyId, projectId, month, outputValue, tax, materialCost, machineCost, machineDeprCost,
                                   laborMgmtCost, laborProjectCost, otherCost, financeFee,
                                   nonprodIncome, nonprodExpense, incomeTax, assessProfit, remark);
}

ApiResult FinanceService::CreateApAccrualRecord(long long companyId, long long projectId, const std::string& vendorName,
                                                const std::string& bizType, double amount, const std::string& bizDate) {
    if (companyId <= 0 || projectId <= 0 || vendorName.empty() || bizType.empty() || !IsDateFormatOk(bizDate)) {
        return ApiResult::Fail(30003, "invalid_params");
    }
    return repo_.CreateApAccrual(companyId, projectId, vendorName, bizType, amount, bizDate);
}

ApiResult FinanceService::CreateApPaymentRecord(long long companyId, long long projectId, const std::string& vendorName,
                                                const std::string& bizType, double amount, const std::string& payDate) {
    if (companyId <= 0 || projectId <= 0 || vendorName.empty() || bizType.empty() || !IsDateFormatOk(payDate)) {
        return ApiResult::Fail(30003, "invalid_params");
    }
    return repo_.CreateApPayment(companyId, projectId, vendorName, bizType, amount, payDate);
}

ApiResult FinanceService::ListCostBenefit(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    if (companyId <= 0 || !IsDateFormatOk(dateFrom) || !IsDateFormatOk(dateTo)) {
        return ApiResult::Fail(30003, "invalid_params");
    }
    return repo_.ListCostBenefit(companyId, dateFrom, dateTo);
}

ApiResult FinanceService::ListApAccrual(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    if (companyId <= 0 || !IsDateFormatOk(dateFrom) || !IsDateFormatOk(dateTo)) {
        return ApiResult::Fail(30003, "invalid_params");
    }
    return repo_.ListApAccrual(companyId, dateFrom, dateTo);
}

ApiResult FinanceService::ListApPayment(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    if (companyId <= 0 || !IsDateFormatOk(dateFrom) || !IsDateFormatOk(dateTo)) {
        return ApiResult::Fail(30003, "invalid_params");
    }
    return repo_.ListApPayment(companyId, dateFrom, dateTo);
}
