#include "service/AuthService.h"
#include <chrono>

AuthService::AuthService() {
    User admin{1, "admin", "admin123", true};
    User finA{2, "finance_a", "finance123", false};
    User finB{3, "finance_b", "finance123", false};

    usersByName_[admin.username] = admin;
    usersByName_[finA.username] = finA;
    usersByName_[finB.username] = finB;

    usersById_[admin.id] = admin;
    usersById_[finA.id] = finA;
    usersById_[finB.id] = finB;

    userCompanyAccess_[finA.id].insert(1);
    userCompanyAccess_[finB.id].insert(2);
}

std::string AuthService::NewToken(long long userId) const {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    return "tk_" + std::to_string(userId) + "_" + std::to_string(now);
}

ApiResult AuthService::Login(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = usersByName_.find(username);
    if (it == usersByName_.end() || it->second.password != password) {
        return ApiResult::Fail(30001, "invalid_username_or_password");
    }

    std::string token = NewToken(it->second.id);
    tokenToUserId_[token] = it->second.id;

    Json data;
    data["token"] = token;
    data["userId"] = it->second.id;
    data["username"] = it->second.username;
    data["isAdmin"] = it->second.isAdmin;
    return ApiResult::Ok(data);
}

bool AuthService::ValidateToken(const std::string& token, long long& outUserId, bool& outIsAdmin) const {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = tokenToUserId_.find(token);
    if (it == tokenToUserId_.end()) return false;

    auto uit = usersById_.find(it->second);
    if (uit == usersById_.end()) return false;

    outUserId = uit->second.id;
    outIsAdmin = uit->second.isAdmin;
    return true;
}

bool AuthService::CanAccessCompany(long long userId, long long companyId) const {
    std::lock_guard<std::mutex> lock(mu_);
    auto uit = usersById_.find(userId);
    if (uit == usersById_.end()) return false;
    if (uit->second.isAdmin) return true;

    auto pit = userCompanyAccess_.find(userId);
    if (pit == userCompanyAccess_.end()) return false;
    return pit->second.count(companyId) > 0;
}

ApiResult AuthService::SetCompanyAccess(long long operatorUserId, bool operatorIsAdmin, long long targetUserId, long long companyId, bool allow) {
    (void)operatorUserId;
    if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
    if (targetUserId <= 0 || companyId <= 0) return ApiResult::Fail(30003, "invalid_params");

    std::lock_guard<std::mutex> lock(mu_);
    if (!usersById_.count(targetUserId)) return ApiResult::Fail(30004, "target_user_not_found");

    if (allow) userCompanyAccess_[targetUserId].insert(companyId);
    else userCompanyAccess_[targetUserId].erase(companyId);

    return ApiResult::Ok(Json{{"targetUserId", targetUserId}, {"companyId", companyId}, {"allow", allow}});
}

ApiResult AuthService::ListPermissions(long long operatorUserId, bool operatorIsAdmin) const {
    if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");

    std::lock_guard<std::mutex> lock(mu_);
    Json users = Json::array();
    for (const auto& kv : usersById_) {
        if (kv.second.isAdmin) continue;

        Json companies = Json::array();
        auto pit = userCompanyAccess_.find(kv.first);
        if (pit != userCompanyAccess_.end()) {
            for (auto cid : pit->second) companies.push_back(cid);
        }

        users.push_back(Json{
            {"userId", kv.second.id},
            {"username", kv.second.username},
            {"companies", companies}
        });
    }

    return ApiResult::Ok(Json{{"operatorUserId", operatorUserId}, {"users", users}});
}
