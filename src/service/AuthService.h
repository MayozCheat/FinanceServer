#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include "core/Result.h"

class AuthService {
public:
    AuthService() {
        User admin{1, "admin", "admin123", true};
        User finA{2, "finance_a", "finance123", false};
        User finB{3, "finance_b", "finance123", false};

        usersByName_[admin.username] = admin;
        usersByName_[finA.username] = finA;
        usersByName_[finB.username] = finB;

        usersById_[admin.id] = admin;
        usersById_[finA.id] = finA;
        usersById_[finB.id] = finB;

        userCompanyPermissions_[finA.id][1] = CompanyPermission{true, true};
        userCompanyPermissions_[finB.id][2] = CompanyPermission{true, true};
    }

    inline ApiResult Login(const std::string& username, const std::string& password) {
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

    inline bool ValidateToken(const std::string& token, long long& outUserId, bool& outIsAdmin) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = tokenToUserId_.find(token);
        if (it == tokenToUserId_.end()) return false;

        auto uit = usersById_.find(it->second);
        if (uit == usersById_.end()) return false;

        outUserId = uit->second.id;
        outIsAdmin = uit->second.isAdmin;
        return true;
    }

    inline bool CanAccessCompany(long long userId, long long companyId) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto uit = usersById_.find(userId);
        if (uit == usersById_.end()) return false;
        if (uit->second.isAdmin) return true;

        auto pit = userCompanyPermissions_.find(userId);
        if (pit == userCompanyPermissions_.end()) return false;
        auto cit = pit->second.find(companyId);
        if (cit == pit->second.end()) return false;
        return cit->second.canRead || cit->second.canWrite;
    }

    inline bool CanModifyCompany(long long userId, long long companyId) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto uit = usersById_.find(userId);
        if (uit == usersById_.end()) return false;
        if (uit->second.isAdmin) return true;

        auto pit = userCompanyPermissions_.find(userId);
        if (pit == userCompanyPermissions_.end()) return false;
        auto cit = pit->second.find(companyId);
        if (cit == pit->second.end()) return false;
        return cit->second.canWrite;
    }



    inline ApiResult SetCompanyAccess(long long operatorUserId, bool operatorIsAdmin, long long targetUserId, long long companyId, bool allow) {
        if (allow) return AddCompanyPermission(operatorUserId, operatorIsAdmin, targetUserId, companyId, true, true);
        return RemoveCompanyPermission(operatorUserId, operatorIsAdmin, targetUserId, companyId, true, true);
    }

    inline ApiResult AddCompanyPermission(long long operatorUserId, bool operatorIsAdmin, long long targetUserId,
                                          long long companyId, bool grantRead, bool grantWrite) {
        (void)operatorUserId;
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
        if (targetUserId <= 0 || companyId <= 0) return ApiResult::Fail(30003, "invalid_params");

        std::lock_guard<std::mutex> lock(mu_);
        auto it = usersById_.find(targetUserId);
        if (it == usersById_.end()) return ApiResult::Fail(30004, "target_user_not_found");
        if (it->second.isAdmin) return ApiResult::Fail(30003, "cannot_edit_admin_permissions");

        CompanyPermission& perm = userCompanyPermissions_[targetUserId][companyId];
        if (grantRead) {
            perm.canRead = true;
        }
        if (grantWrite) {
            perm.canWrite = true;
            perm.canRead = true;
        }

        if (!perm.canRead && !perm.canWrite) {
            userCompanyPermissions_[targetUserId].erase(companyId);
        }

        return ApiResult::Ok(Json{{"targetUserId", targetUserId}, {"companyId", companyId}, {"canRead", perm.canRead}, {"canWrite", perm.canWrite}});
    }

    inline ApiResult RemoveCompanyPermission(long long operatorUserId, bool operatorIsAdmin, long long targetUserId,
                                             long long companyId, bool removeRead, bool removeWrite) {
        (void)operatorUserId;
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
        if (targetUserId <= 0 || companyId <= 0) return ApiResult::Fail(30003, "invalid_params");

        std::lock_guard<std::mutex> lock(mu_);
        auto it = usersById_.find(targetUserId);
        if (it == usersById_.end()) return ApiResult::Fail(30004, "target_user_not_found");
        if (it->second.isAdmin) return ApiResult::Fail(30003, "cannot_edit_admin_permissions");

        auto pit = userCompanyPermissions_.find(targetUserId);
        if (pit == userCompanyPermissions_.end() || pit->second.count(companyId) == 0) {
            return ApiResult::Ok(Json{{"targetUserId", targetUserId}, {"companyId", companyId}, {"canRead", false}, {"canWrite", false}});
        }

        CompanyPermission& perm = pit->second[companyId];
        if (removeRead) {
            perm.canRead = false;
            perm.canWrite = false;
        }
        if (removeWrite) {
            perm.canWrite = false;
        }
        if (perm.canWrite && !perm.canRead) {
            perm.canRead = true;
        }

        bool canRead = perm.canRead;
        bool canWrite = perm.canWrite;
        if (!canRead && !canWrite) {
            pit->second.erase(companyId);
        }

        return ApiResult::Ok(Json{{"targetUserId", targetUserId}, {"companyId", companyId}, {"canRead", canRead}, {"canWrite", canWrite}});
    }

    inline ApiResult ListUsers(long long operatorUserId, bool operatorIsAdmin) const {
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");

        std::lock_guard<std::mutex> lock(mu_);
        Json users = Json::array();
        for (const auto& kv : usersById_) {
            Json companies = Json::array();
            if (!kv.second.isAdmin) {
                auto pit = userCompanyPermissions_.find(kv.first);
                if (pit != userCompanyPermissions_.end()) {
                    for (const auto& pkv : pit->second) {
                        if (!pkv.second.canRead && !pkv.second.canWrite) continue;
                        companies.push_back(Json{{"companyId", pkv.first}, {"canRead", pkv.second.canRead}, {"canWrite", pkv.second.canWrite}});
                    }
                }
            }
            users.push_back(Json{{"userId", kv.second.id}, {"username", kv.second.username}, {"isAdmin", kv.second.isAdmin}, {"companies", companies}});
        }

        return ApiResult::Ok(Json{{"operatorUserId", operatorUserId}, {"users", users}});
    }

    inline ApiResult DeleteUser(long long operatorUserId, bool operatorIsAdmin, long long targetUserId) {
        (void)operatorUserId;
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
        if (targetUserId <= 0 || targetUserId == 1) return ApiResult::Fail(30003, "invalid_params");

        std::lock_guard<std::mutex> lock(mu_);
        auto it = usersById_.find(targetUserId);
        if (it == usersById_.end()) return ApiResult::Fail(30004, "target_user_not_found");
        if (it->second.isAdmin) return ApiResult::Fail(30003, "cannot_delete_admin");

        usersByName_.erase(it->second.username);
        usersById_.erase(it);
        userCompanyPermissions_.erase(targetUserId);

        for (auto t = tokenToUserId_.begin(); t != tokenToUserId_.end();) {
            if (t->second == targetUserId) t = tokenToUserId_.erase(t);
            else ++t;
        }

        return ApiResult::Ok(Json{{"targetUserId", targetUserId}, {"deleted", true}});
    }

    inline ApiResult CreateUser(long long operatorUserId, bool operatorIsAdmin, long long userId,
                                const std::string& username, const std::string& password, bool isAdmin) {
        (void)operatorUserId;
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
        if (userId <= 0 || username.empty() || password.empty()) return ApiResult::Fail(30003, "invalid_params");

        std::lock_guard<std::mutex> lock(mu_);
        if (usersById_.count(userId) > 0 || usersByName_.count(username) > 0) {
            return ApiResult::Fail(30007, "user_already_exists");
        }

        User u{userId, username, password, isAdmin};
        usersById_[u.id] = u;
        usersByName_[u.username] = u;

        return ApiResult::Ok(Json{{"userId", u.id}, {"username", u.username}, {"isAdmin", u.isAdmin}});
    }

    inline ApiResult ResetPassword(long long operatorUserId, bool operatorIsAdmin, long long targetUserId,
                                   const std::string& newPassword) {
        (void)operatorUserId;
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
        if (targetUserId <= 0 || newPassword.empty()) return ApiResult::Fail(30003, "invalid_params");

        std::lock_guard<std::mutex> lock(mu_);
        auto it = usersById_.find(targetUserId);
        if (it == usersById_.end()) return ApiResult::Fail(30004, "target_user_not_found");

        it->second.password = newPassword;
        usersByName_[it->second.username] = it->second;
        return ApiResult::Ok(Json{{"targetUserId", targetUserId}});
    }

    inline ApiResult WhoAmI(long long userId, bool isAdmin) const {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = usersById_.find(userId);
        if (it == usersById_.end()) return ApiResult::Fail(30006, "invalid_token");

        Json companies = Json::array();
        if (!isAdmin) {
            auto pit = userCompanyPermissions_.find(userId);
            if (pit != userCompanyPermissions_.end()) {
                for (const auto& pkv : pit->second) {
                    if (pkv.second.canRead || pkv.second.canWrite) companies.push_back(pkv.first);
                }
            }
        }

        Json data{{"userId", it->second.id}, {"username", it->second.username}, {"isAdmin", it->second.isAdmin}, {"companies", companies}};
        return ApiResult::Ok(data);
    }

    inline ApiResult ListPermissions(long long operatorUserId, bool operatorIsAdmin) const {
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");

        std::lock_guard<std::mutex> lock(mu_);
        Json users = Json::array();
        for (const auto& kv : usersById_) {
            if (kv.second.isAdmin) continue;

            Json companies = Json::array();
            auto pit = userCompanyPermissions_.find(kv.first);
            if (pit != userCompanyPermissions_.end()) {
                for (const auto& pkv : pit->second) {
                    if (!pkv.second.canRead && !pkv.second.canWrite) continue;
                    companies.push_back(Json{{"companyId", pkv.first}, {"canRead", pkv.second.canRead}, {"canWrite", pkv.second.canWrite}});
                }
            }

            users.push_back(Json{{"userId", kv.second.id}, {"username", kv.second.username}, {"companies", companies}});
        }

        return ApiResult::Ok(Json{{"operatorUserId", operatorUserId}, {"users", users}});
    }

private:
    struct User {
        long long id{0};
        std::string username;
        std::string password;
        bool isAdmin{false};
    };

    struct CompanyPermission {
        bool canRead{false};
        bool canWrite{false};
    };

    inline std::string NewToken(long long userId) const {
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "tk_" + std::to_string(userId) + "_" + std::to_string(now);
    }

    mutable std::mutex mu_;
    std::unordered_map<std::string, User> usersByName_;
    std::unordered_map<long long, User> usersById_;
    std::unordered_map<std::string, long long> tokenToUserId_;
    std::unordered_map<long long, std::unordered_map<long long, CompanyPermission>> userCompanyPermissions_;
};
