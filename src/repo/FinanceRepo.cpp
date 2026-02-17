#include "repo/FinanceRepo.h"
#include "core/Logger.h"
#include <sstream>
#include <algorithm>
#include <cctype>

std::string FinanceRepo::EscapeSql(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\'') out += "\\'";
        else out.push_back(c);
    }
    return out;
}

bool FinanceRepo::IsTableMissingError(const std::string& err) {
    std::string low = err;
    std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) { return std::tolower(c); });
    return low.find("doesn't exist") != std::string::npos ||
           low.find("1146") != std::string::npos ||
           low.find("unknown table") != std::string::npos;
}

ApiResult FinanceRepo::ListCompanies() {
    std::vector<std::string> cols;
    std::vector<Db::Row> rows;
    std::string err;
    if (!db_.Query("SELECT id, name FROM companies ORDER BY id;", cols, rows, err)) {
        if (IsTableMissingError(err)) return ApiResult::Fail(20021, "table_not_found_companies");
        return ApiResult::Fail(20001, "db_query_failed");
    }

    Json arr = Json::array();
    for (const auto& r : rows) {
        arr.push_back(Json{{"id", std::stoll(r.cols[0])}, {"name", r.cols[1]}});
    }
    return ApiResult::Ok(Json{{"rows", arr}});
}

ApiResult FinanceRepo::ListProjects(long long companyId) {
    std::ostringstream oss;
    oss << "SELECT id, company_id, name FROM projects ";
    if (companyId > 0) oss << "WHERE company_id=" << companyId << " ";
    oss << "ORDER BY id;";

    std::vector<std::string> cols;
    std::vector<Db::Row> rows;
    std::string err;
    if (!db_.Query(oss.str(), cols, rows, err)) {
        if (IsTableMissingError(err)) return ApiResult::Fail(20022, "table_not_found_projects");
        return ApiResult::Fail(20001, "db_query_failed");
    }

    Json arr = Json::array();
    for (const auto& r : rows) {
        arr.push_back(Json{{"id", std::stoll(r.cols[0])}, {"companyId", std::stoll(r.cols[1])}, {"name", r.cols[2]}});
    }
    return ApiResult::Ok(Json{{"rows", arr}});
}

ApiResult FinanceRepo::CreateCompany(long long companyId, const std::string& name) {
    std::ostringstream oss;
    oss << "INSERT INTO companies(id, name) VALUES (" << companyId << ", '" << EscapeSql(name) << "');";

    int affected = 0;
    std::string err;
    if (!db_.Execute(oss.str(), affected, err)) {
        return ApiResult::Fail(20004, "db_write_failed");
    }
    return ApiResult::Ok(Json{{"affectedRows", affected}, {"id", companyId}});
}

ApiResult FinanceRepo::CreateProject(long long projectId, long long companyId, const std::string& name) {
    std::ostringstream oss;
    oss << "INSERT INTO projects(id, company_id, name) VALUES ("
        << projectId << ", " << companyId << ", '" << EscapeSql(name) << "');";

    int affected = 0;
    std::string err;
    if (!db_.Execute(oss.str(), affected, err)) {
        return ApiResult::Fail(20004, "db_write_failed");
    }
    return ApiResult::Ok(Json{{"affectedRows", affected}, {"id", projectId}});
}

ApiResult FinanceRepo::UpsertCostBenefit(long long companyId, long long projectId, const std::string& month,
                                         double outputValue, double tax, double materialCost,
                                         double machineCost, double machineDeprCost, double laborMgmtCost,
                                         double laborProjectCost, double otherCost, double financeFee,
                                         double nonprodIncome, double nonprodExpense, double incomeTax,
                                         double assessProfit, const std::string& remark) {
    std::ostringstream oss;
    oss << "INSERT INTO cost_benefit_monthly("
        << "company_id, project_id, month, output_value, tax, material_cost, machine_cost, machine_depr_cost,"
        << "labor_mgmt_cost, labor_project_cost, other_cost, finance_fee, nonprod_income, nonprod_expense, income_tax, assess_profit, remark"
        << ") VALUES ("
        << companyId << ", " << projectId << ", '" << EscapeSql(month) << "', "
        << outputValue << ", " << tax << ", " << materialCost << ", " << machineCost << ", " << machineDeprCost << ", "
        << laborMgmtCost << ", " << laborProjectCost << ", " << otherCost << ", " << financeFee << ", "
        << nonprodIncome << ", " << nonprodExpense << ", " << incomeTax << ", " << assessProfit << ", '" << EscapeSql(remark) << "');";

    int affected = 0;
    std::string err;
    if (!db_.Execute(oss.str(), affected, err)) {
        return ApiResult::Fail(20004, "db_write_failed");
    }
    return ApiResult::Ok(Json{{"affectedRows", affected}});
}

ApiResult FinanceRepo::CreateApAccrual(long long companyId, long long projectId, const std::string& vendorName,
                                       const std::string& bizType, double amount, const std::string& bizDate) {
    std::ostringstream oss;
    oss << "INSERT INTO ap_accrual(company_id, project_id, vendor_name, biz_type, amount, biz_date) VALUES ("
        << companyId << ", " << projectId << ", '" << EscapeSql(vendorName) << "', '"
        << EscapeSql(bizType) << "', " << amount << ", '" << EscapeSql(bizDate) << "');";

    int affected = 0;
    std::string err;
    if (!db_.Execute(oss.str(), affected, err)) {
        return ApiResult::Fail(20004, "db_write_failed");
    }
    return ApiResult::Ok(Json{{"affectedRows", affected}});
}

ApiResult FinanceRepo::CreateApPayment(long long companyId, long long projectId, const std::string& vendorName,
                                       const std::string& bizType, double amount, const std::string& payDate) {
    std::ostringstream oss;
    oss << "INSERT INTO ap_payment(company_id, project_id, vendor_name, biz_type, amount, pay_date) VALUES ("
        << companyId << ", " << projectId << ", '" << EscapeSql(vendorName) << "', '"
        << EscapeSql(bizType) << "', " << amount << ", '" << EscapeSql(payDate) << "');";

    int affected = 0;
    std::string err;
    if (!db_.Execute(oss.str(), affected, err)) {
        return ApiResult::Fail(20004, "db_write_failed");
    }
    return ApiResult::Ok(Json{{"affectedRows", affected}});
}

ApiResult FinanceRepo::ListApAccrual(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    std::ostringstream oss;
    oss << "SELECT id, company_id, project_id, vendor_name, biz_type, amount, biz_date FROM ap_accrual "
        << "WHERE company_id=" << companyId << " AND biz_date>='" << EscapeSql(dateFrom) << "' AND biz_date<='" << EscapeSql(dateTo) << "' "
        << "ORDER BY id DESC;";

    std::vector<std::string> cols;
    std::vector<Db::Row> rows;
    std::string err;
    if (!db_.Query(oss.str(), cols, rows, err)) {
        return ApiResult::Fail(20001, "db_query_failed");
    }

    Json arr = Json::array();
    for (const auto& r : rows) {
        arr.push_back(Json{{"id", std::stoll(r.cols[0])}, {"companyId", std::stoll(r.cols[1])}, {"projectId", std::stoll(r.cols[2])},
                           {"vendorName", r.cols[3]}, {"bizType", r.cols[4]}, {"amount", std::stod(r.cols[5])}, {"bizDate", r.cols[6]}});
    }
    return ApiResult::Ok(Json{{"rows", arr}});
}

ApiResult FinanceRepo::ListApPayment(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    std::ostringstream oss;
    oss << "SELECT id, company_id, project_id, vendor_name, biz_type, amount, pay_date FROM ap_payment "
        << "WHERE company_id=" << companyId << " AND pay_date>='" << EscapeSql(dateFrom) << "' AND pay_date<='" << EscapeSql(dateTo) << "' "
        << "ORDER BY id DESC;";

    std::vector<std::string> cols;
    std::vector<Db::Row> rows;
    std::string err;
    if (!db_.Query(oss.str(), cols, rows, err)) {
        return ApiResult::Fail(20001, "db_query_failed");
    }

    Json arr = Json::array();
    for (const auto& r : rows) {
        arr.push_back(Json{{"id", std::stoll(r.cols[0])}, {"companyId", std::stoll(r.cols[1])}, {"projectId", std::stoll(r.cols[2])},
                           {"vendorName", r.cols[3]}, {"bizType", r.cols[4]}, {"amount", std::stod(r.cols[5])}, {"payDate", r.cols[6]}});
    }
    return ApiResult::Ok(Json{{"rows", arr}});
}
