#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include "user.h"
#include "userManager.h"
#include "jwtService.h"
#include <optional>
#include <string>
#include <memory>

struct LoginResult {
    bool success;
    std::string token;
    std::string userId;
    std::string email;
    std::string message;
};

struct RegisterResult {
    bool success;
    std::string userId;
    std::string message;
};

struct AuthResult {
    bool authenticated;
    std::string userId;
    std::string email;
    std::string message;
};

class AuthService {
public:
    AuthService(std::shared_ptr<UserManager> userManager, std::shared_ptr<JwtService> jwtService);

    // Registration
    RegisterResult registerUser(const std::string& email, const std::string& password);

    // Login
    LoginResult login(const std::string& email, const std::string& password);

    // Token validation
    AuthResult validateToken(const std::string& token);

    // Password verification
    bool verifyPassword(const User& user, const std::string& password);

    // User retrieval
    std::optional<User> getUserById(const std::string& userId);
    std::optional<User> getUserByEmail(const std::string& email);

    // User management
    bool updateUser(const User& user);
    bool deactivateUser(const std::string& userId);
    bool reactivateUser(const std::string& userId);

    // Password management
    bool changePassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword);

private:
    std::shared_ptr<UserManager> userManager_;
    std::shared_ptr<JwtService> jwtService_;

    // Validation helpers
    bool validateEmail(const std::string& email) const;
    bool validatePassword(const std::string& password) const;
};

#endif // AUTH_SERVICE_H
