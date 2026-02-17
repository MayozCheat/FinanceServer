#include "repo/ReportRepo.h"
#include "core/Logger.h"
#include <sstream>
#include <unordered_map>

ApiResult ReportRepo::QueryCostBenefit(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    std::ostringstream oss;
    oss
        << "SELECT c.name AS companyName, p.name AS projectName, "
        << "cb.output_value AS outputValue, cb.tax AS tax, cb.material_cost AS materialCost, "
        << "cb.machine_cost AS machineCost, cb.machine_depr_cost AS machineDeprCost, "
        << "cb.labor_mgmt_cost AS laborMgmtCost, cb.labor_project_cost AS laborProjectCost, "
        << "cb.other_cost AS otherCost, cb.finance_fee AS financeFee, "
        << "(cb.tax + cb.material_cost + cb.machine_cost + cb.labor_mgmt_cost + cb.labor_project_cost + cb.other_cost) AS totalCost, "
        << "cb.nonprod_income AS nonprodIncome, cb.nonprod_expense AS nonprodExpense, "
        << "((cb.output_value + cb.nonprod_income) - (cb.tax + cb.material_cost + cb.machine_cost + cb.labor_mgmt_cost + cb.labor_project_cost + cb.other_cost + cb.nonprod_expense)) AS profit, "
        << "cb.income_tax AS incomeTax, cb.assess_profit AS assessProfit, cb.remark AS remark "
        << "FROM cost_benefit_monthly cb "
        << "JOIN companies c ON c.id = cb.company_id "
        << "JOIN projects p ON p.id = cb.project_id "
        << "WHERE cb.company_id=" << companyId << " "
        << "AND cb.month>='" << dateFrom << "' AND cb.month<='" << dateTo << "' "
        << "ORDER BY p.id;";

    std::vector<std::string> cols;
    std::vector<Db::Row> rows;
    std::string err;

    if (!db_.Query(oss.str(), cols, rows, err)) {
        Logger::Instance().Error("【报表】成本效益查询失败：" + err);
        return ApiResult::Fail(20001, "db_query_failed");
    }

    Json arr = Json::array();
    for (const auto& r : rows) {
        Json item;
        for (size_t i = 0; i < cols.size(); i++) item[cols[i]] = r.cols[i];
        arr.push_back(item);
    }
    return ApiResult::Ok(Json{{"rows", arr}});
}

ApiResult ReportRepo::QueryApSummary(long long companyId, const std::string& dateFrom, const std::string& dateTo) {
    // 1) 挂账聚合
    std::ostringstream acc;
    acc
        << "SELECT p.id AS projectId, p.name AS projectName, "
        << "a.vendor_name AS vendorName, a.biz_type AS bizType, "
        << "SUM(a.amount) AS accrualTotal "
        << "FROM ap_accrual a JOIN projects p ON p.id=a.project_id "
        << "WHERE a.company_id=" << companyId << " "
        << "AND a.biz_date>='" << dateFrom << "' AND a.biz_date<='" << dateTo << "' "
        << "GROUP BY p.id, p.name, a.vendor_name, a.biz_type "
        << "ORDER BY p.id;";

    std::vector<std::string> accCols;
    std::vector<Db::Row> accRows;
    std::string err;
    if (!db_.Query(acc.str(), accCols, accRows, err)) {
        Logger::Instance().Error("【报表】挂账汇总查询失败：" + err);
        return ApiResult::Fail(20002, "db_query_failed");
    }

    // 2) 付款聚合
    std::ostringstream pay;
    pay
        << "SELECT p.id AS projectId, "
        << "a.vendor_name AS vendorName, a.biz_type AS bizType, "
        << "SUM(a.amount) AS paidTotal "
        << "FROM ap_payment a JOIN projects p ON p.id=a.project_id "
        << "WHERE a.company_id=" << companyId << " "
        << "AND a.pay_date>='" << dateFrom << "' AND a.pay_date<='" << dateTo << "' "
        << "GROUP BY p.id, a.vendor_name, a.biz_type;";

    std::vector<std::string> payCols;
    std::vector<Db::Row> payRows;
    if (!db_.Query(pay.str(), payCols, payRows, err)) {
        Logger::Instance().Error("【报表】付款汇总查询失败：" + err);
        return ApiResult::Fail(20003, "db_query_failed");
    }

    // paidMap：projectId|vendor|bizType -> paidTotal
    std::unordered_map<std::string, double> paidMap;
    for (const auto& r : payRows) {
        std::string key = r.cols[0] + "|" + r.cols[1] + "|" + r.cols[2];
        paidMap[key] = std::stod(r.cols[3]);
    }

    // project 聚合
    struct Agg { std::string name; Json vendors=Json::array(); double acc=0, paid=0; };
    std::unordered_map<std::string, Agg> proj;
    std::vector<std::string> order;

    for (const auto& r : accRows) {
        std::string projectId = r.cols[0];
        std::string projectName = r.cols[1];
        std::string vendor = r.cols[2];
        std::string bizType = r.cols[3];
        double accrualTotal = std::stod(r.cols[4]);

        std::string key = projectId + "|" + vendor + "|" + bizType;
        double paidTotal = paidMap.count(key) ? paidMap[key] : 0.0;
        double balance = accrualTotal - paidTotal;

        if (!proj.count(projectId)) { proj[projectId] = Agg{projectName}; order.push_back(projectId); }

        Json v;
        v["vendorName"] = vendor;
        v["bizType"] = bizType;
        v["accrualTotal"] = accrualTotal;
        v["paidTotal"] = paidTotal;
        v["balance"] = balance;

        proj[projectId].vendors.push_back(v);
        proj[projectId].acc += accrualTotal;
        proj[projectId].paid += paidTotal;
    }

    Json projects = Json::array();
    double gAcc=0, gPaid=0;
    for (auto& pid : order) {
        auto& ag = proj[pid];
        Json p;
        p["projectName"] = ag.name;
        p["vendors"] = ag.vendors;
        p["subtotal"] = Json{
            {"accrualTotal", ag.acc},
            {"paidTotal", ag.paid},
            {"balance", ag.acc - ag.paid}
        };
        projects.push_back(p);
        gAcc += ag.acc;
        gPaid += ag.paid;
    }

    Json data;
    data["projects"] = projects;
    data["grandTotal"] = Json{
        {"accrualTotal", gAcc},
        {"paidTotal", gPaid},
        {"balance", gAcc - gPaid}
    };

    return ApiResult::Ok(data);
}
