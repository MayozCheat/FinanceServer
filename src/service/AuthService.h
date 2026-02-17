#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
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
    AuthService();

    ApiResult Login(const std::string& username, const std::string& password);
    bool ValidateToken(const std::string& token, long long& outUserId, bool& outIsAdmin) const;
    bool CanAccessCompany(long long userId, long long companyId) const;

    ApiResult SetCompanyAccess(long long operatorUserId, bool operatorIsAdmin, long long targetUserId, long long companyId, bool allow);
    ApiResult ListPermissions(long long operatorUserId, bool operatorIsAdmin) const;

private:
    struct User {
        long long id{0};
        std::string username;
        std::string password;
        bool isAdmin{false};
    };

    std::string NewToken(long long userId) const;

    mutable std::mutex mu_;
    std::unordered_map<std::string, User> usersByName_;
    std::unordered_map<long long, User> usersById_;
    std::unordered_map<std::string, long long> tokenToUserId_;
    std::unordered_map<long long, std::unordered_set<long long>> userCompanyAccess_;
};
