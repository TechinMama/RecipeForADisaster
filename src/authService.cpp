#include "authService.h"
#include <regex>
#include <iostream>

AuthService::AuthService(std::shared_ptr<UserManager> userManager, std::shared_ptr<JwtService> jwtService)
    : userManager_(std::move(userManager)), jwtService_(std::move(jwtService)) {
    if (!userManager_) {
        throw std::invalid_argument("UserManager cannot be null");
    }
    if (!jwtService_) {
        throw std::invalid_argument("JwtService cannot be null");
    }
}

RegisterResult AuthService::registerUser(const std::string& email, const std::string& password) {
    RegisterResult result{false, "", ""};

    // Validate email format
    if (!validateEmail(email)) {
        result.message = "Invalid email format";
        return result;
    }

    // Validate password strength
    if (!validatePassword(password)) {
        result.message = "Password must be at least 8 characters long and contain uppercase, lowercase, and numbers";
        return result;
    }

    // Check if user already exists
    auto existingUser = userManager_->findUserByEmail(email);
    if (existingUser.has_value()) {
        result.message = "User with this email already exists";
        return result;
    }

    // Create new user
    try {
        User newUser(email, password);
        if (userManager_->createUser(newUser)) {
            result.success = true;
            result.userId = newUser.getId();
            result.message = "User registered successfully";
        } else {
            result.message = "Failed to create user in database";
        }
    } catch (const std::exception& ex) {
        result.message = std::string("Registration error: ") + ex.what();
        std::cerr << result.message << std::endl;
    }

    return result;
}

LoginResult AuthService::login(const std::string& email, const std::string& password) {
    LoginResult result{false, "", "", "", ""};

    try {
        // Find user by email
        auto userOpt = userManager_->findUserByEmail(email);
        if (!userOpt.has_value()) {
            result.message = "Invalid email or password";
            return result;
        }

        const auto& user = userOpt.value();

        // Check if user is active
        if (!user.isActive()) {
            result.message = "Account is deactivated";
            return result;
        }

        // Verify password
        if (!verifyPassword(user, password)) {
            result.message = "Invalid email or password";
            return result;
        }

        // Generate JWT token
        std::string token = jwtService_->generateToken(user);

        result.success = true;
        result.token = token;
        result.userId = user.getId();
        result.email = user.getEmail();
        result.message = "Login successful";

    } catch (const std::exception& ex) {
        result.message = std::string("Login error: ") + ex.what();
        std::cerr << result.message << std::endl;
    }

    return result;
}

AuthResult AuthService::validateToken(const std::string& token) {
    AuthResult result{false, "", "", ""};

    try {
        auto claimsOpt = jwtService_->validateToken(token);
        if (!claimsOpt.has_value()) {
            result.message = "Invalid or expired token";
            return result;
        }

        const auto& claims = claimsOpt.value();

        // Verify user still exists and is active
        auto userOpt = userManager_->findUserById(claims.subject);
        if (!userOpt.has_value()) {
            result.message = "User not found";
            return result;
        }

        const auto& user = userOpt.value();
        if (!user.isActive()) {
            result.message = "User account is deactivated";
            return result;
        }

        result.authenticated = true;
        result.userId = claims.subject;
        result.email = claims.email;
        result.message = "Token validated successfully";

    } catch (const std::exception& ex) {
        result.message = std::string("Token validation error: ") + ex.what();
        std::cerr << result.message << std::endl;
    }

    return result;
}

bool AuthService::verifyPassword(const User& user, const std::string& password) {
    return user.verifyPassword(password);
}

std::optional<User> AuthService::getUserById(const std::string& userId) {
    return userManager_->findUserById(userId);
}

std::optional<User> AuthService::getUserByEmail(const std::string& email) {
    return userManager_->findUserByEmail(email);
}

bool AuthService::updateUser(const User& user) {
    try {
        return userManager_->updateUser(user);
    } catch (const std::exception& ex) {
        std::cerr << "Error updating user: " << ex.what() << std::endl;
        return false;
    }
}

bool AuthService::deactivateUser(const std::string& userId) {
    try {
        auto userOpt = userManager_->findUserById(userId);
        if (!userOpt.has_value()) {
            return false;
        }

        User user = userOpt.value();
        user.setActive(false);
        return userManager_->updateUser(user);
    } catch (const std::exception& ex) {
        std::cerr << "Error deactivating user: " << ex.what() << std::endl;
        return false;
    }
}

bool AuthService::reactivateUser(const std::string& userId) {
    try {
        auto userOpt = userManager_->findUserById(userId);
        if (!userOpt.has_value()) {
            return false;
        }

        User user = userOpt.value();
        user.setActive(true);
        return userManager_->updateUser(user);
    } catch (const std::exception& ex) {
        std::cerr << "Error reactivating user: " << ex.what() << std::endl;
        return false;
    }
}

bool AuthService::changePassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword) {
    try {
        // Validate new password
        if (!validatePassword(newPassword)) {
            std::cerr << "New password does not meet requirements" << std::endl;
            return false;
        }

        // Get user
        auto userOpt = userManager_->findUserById(userId);
        if (!userOpt.has_value()) {
            return false;
        }

        User user = userOpt.value();

        // Verify old password
        if (!verifyPassword(user, oldPassword)) {
            std::cerr << "Old password is incorrect" << std::endl;
            return false;
        }

        // Set new password hash
        std::string newPasswordHash = user.hashPassword(newPassword);
        user.setPasswordHash(newPasswordHash);
        user.setUpdatedAt(std::chrono::system_clock::now());
        return userManager_->updateUser(user);

    } catch (const std::exception& ex) {
        std::cerr << "Error changing password: " << ex.what() << std::endl;
        return false;
    }
}

bool AuthService::validateEmail(const std::string& email) const {
    // Basic email validation regex
    const std::regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, emailPattern);
}

bool AuthService::validatePassword(const std::string& password) const {
    if (password.length() < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        if (std::islower(c)) hasLower = true;
        if (std::isdigit(c)) hasDigit = true;
    }

    return hasUpper && hasLower && hasDigit;
}
