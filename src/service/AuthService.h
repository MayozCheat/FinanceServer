#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include "core/Result.h"

/**
 * @brief AuthService：认证与权限服务（第一版：内存实现）
 * 负责：
 * - 登录（用户名/密码 -> token）
 * - token 鉴权
 * - 公司级权限校验
 * - 管理员设置用户公司权限
 */
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

        userCompanyAccess_[finA.id].insert(1);
        userCompanyAccess_[finB.id].insert(2);
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

        auto pit = userCompanyAccess_.find(userId);
        if (pit == userCompanyAccess_.end()) return false;
        return pit->second.count(companyId) > 0;
    }

    inline ApiResult SetCompanyAccess(long long operatorUserId, bool operatorIsAdmin, long long targetUserId, long long companyId, bool allow) {
        (void)operatorUserId;
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");
        if (targetUserId <= 0 || companyId <= 0) return ApiResult::Fail(30003, "invalid_params");

        std::lock_guard<std::mutex> lock(mu_);
        if (!usersById_.count(targetUserId)) return ApiResult::Fail(30004, "target_user_not_found");

        if (allow) userCompanyAccess_[targetUserId].insert(companyId);
        else userCompanyAccess_[targetUserId].erase(companyId);

        return ApiResult::Ok(Json{{"targetUserId", targetUserId}, {"companyId", companyId}, {"allow", allow}});
    }

    inline ApiResult ListUsers(long long operatorUserId, bool operatorIsAdmin) const {
        if (!operatorIsAdmin) return ApiResult::Fail(30002, "forbidden");

        std::lock_guard<std::mutex> lock(mu_);
        Json users = Json::array();
        for (const auto& kv : usersById_) {
            users.push_back(Json{{"userId", kv.second.id}, {"username", kv.second.username}, {"isAdmin", kv.second.isAdmin}});
        }

        return ApiResult::Ok(Json{{"operatorUserId", operatorUserId}, {"users", users}});
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
            auto pit = userCompanyAccess_.find(userId);
            if (pit != userCompanyAccess_.end()) {
                for (auto cid : pit->second) companies.push_back(cid);
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

private:
    struct User {
        long long id{0};
        std::string username;
        std::string password;
        bool isAdmin{false};
    };

    inline std::string NewToken(long long userId) const {
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "tk_" + std::to_string(userId) + "_" + std::to_string(now);
    }

    mutable std::mutex mu_;
    std::unordered_map<std::string, User> usersByName_;
    std::unordered_map<long long, User> usersById_;
    std::unordered_map<std::string, long long> tokenToUserId_;
    std::unordered_map<long long, std::unordered_set<long long>> userCompanyAccess_;
};
