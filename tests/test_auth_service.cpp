#include <gtest/gtest.h>
#include "authService.h"
#include "userManager.h"
#include "jwtService.h"
#include <memory>
#include <filesystem>
#include <sqlite3.h>

class AuthServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test database
        testDbPath = "test_auth_service.db";
        
        // Remove existing test database if it exists
        std::filesystem::remove(testDbPath);

        // Open database
        int rc = sqlite3_open(testDbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Cannot open test database");
        }

        // Create users table
        const char* createTableSql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id TEXT PRIMARY KEY,
                email TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                is_active INTEGER NOT NULL DEFAULT 1,
                name TEXT,
                bio TEXT,
                avatar_url TEXT,
                preferences TEXT,
                privacy_settings TEXT
            )
        )";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, createTableSql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::string error = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            throw std::runtime_error("Failed to create users table: " + error);
        }

        // Create UserManager and JwtService
        userManager = std::make_shared<UserManager>(db);
        
        JwtService::Config jwtConfig;
        jwtConfig.secret = "test-secret-key-for-auth-service";
        jwtConfig.issuer = "test-issuer";
        jwtConfig.audience = "test-audience";
        jwtConfig.accessTokenLifetime = std::chrono::hours(1);
        jwtService = std::make_shared<JwtService>(jwtConfig);

        // Create AuthService
        authService = std::make_shared<AuthService>(userManager, jwtService);
    }

    void TearDown() override {
        authService.reset();
        jwtService.reset();
        userManager.reset();
        
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        
        // Clean up test database
        std::filesystem::remove(testDbPath);
    }

    std::string testDbPath;
    sqlite3* db = nullptr;
    std::shared_ptr<UserManager> userManager;
    std::shared_ptr<JwtService> jwtService;
    std::shared_ptr<AuthService> authService;
};

TEST_F(AuthServiceTest, RegisterUserSuccess) {
    auto result = authService->registerUser("test@example.com", "Password123");
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.userId.empty());
    EXPECT_EQ(result.message, "User registered successfully");
    
    // Verify user can be found
    auto foundUser = authService->getUserByEmail("test@example.com");
    ASSERT_TRUE(foundUser.has_value()) << "User should be found after registration";
    EXPECT_EQ(foundUser->getEmail(), "test@example.com");
}

TEST_F(AuthServiceTest, RegisterUserInvalidEmail) {
    auto result = authService->registerUser("invalid-email", "Password123");
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.userId.empty());
    EXPECT_EQ(result.message, "Invalid email format");
}

TEST_F(AuthServiceTest, RegisterUserWeakPassword) {
    auto result = authService->registerUser("test@example.com", "weak");
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.userId.empty());
    EXPECT_TRUE(result.message.find("Password must be") != std::string::npos);
}

TEST_F(AuthServiceTest, RegisterUserDuplicateEmail) {
    // Register first user
    authService->registerUser("test@example.com", "Password123");
    
    // Try to register with same email
    auto result = authService->registerUser("test@example.com", "Password456");
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "User with this email already exists");
}

TEST_F(AuthServiceTest, LoginSuccess) {
    // Register user first
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    ASSERT_TRUE(registerResult.success) << "Registration failed: " << registerResult.message;
    
    // Verify user exists
    auto foundUser = authService->getUserByEmail("test@example.com");
    ASSERT_TRUE(foundUser.has_value()) << "User should exist after registration";
    
    // Verify password manually
    ASSERT_TRUE(foundUser->verifyPassword("Password123")) << "Password verification should succeed";
    
    // Login
    auto result = authService->login("test@example.com", "Password123");
    
    EXPECT_TRUE(result.success) << "Login failed: " << result.message;
    EXPECT_FALSE(result.token.empty());
    EXPECT_FALSE(result.userId.empty());
    EXPECT_EQ(result.email, "test@example.com");
    EXPECT_EQ(result.message, "Login successful");
}

TEST_F(AuthServiceTest, LoginInvalidEmail) {
    auto result = authService->login("nonexistent@example.com", "Password123");
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.token.empty());
    EXPECT_EQ(result.message, "Invalid email or password");
}

TEST_F(AuthServiceTest, LoginInvalidPassword) {
    // Register user first
    authService->registerUser("test@example.com", "Password123");
    
    // Try to login with wrong password
    auto result = authService->login("test@example.com", "WrongPassword456");
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.token.empty());
    EXPECT_EQ(result.message, "Invalid email or password");
}

TEST_F(AuthServiceTest, LoginDeactivatedUser) {
    // Register and deactivate user
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    authService->deactivateUser(registerResult.userId);
    
    // Try to login
    auto result = authService->login("test@example.com", "Password123");
    
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "Account is deactivated");
}

TEST_F(AuthServiceTest, ValidateTokenSuccess) {
    // Register and login
    authService->registerUser("test@example.com", "Password123");
    auto loginResult = authService->login("test@example.com", "Password123");
    
    // Validate token
    auto result = authService->validateToken(loginResult.token);
    
    EXPECT_TRUE(result.authenticated);
    EXPECT_EQ(result.email, "test@example.com");
    EXPECT_EQ(result.message, "Token validated successfully");
}

TEST_F(AuthServiceTest, ValidateTokenInvalid) {
    auto result = authService->validateToken("invalid.token.here");
    
    EXPECT_FALSE(result.authenticated);
    EXPECT_EQ(result.message, "Invalid or expired token");
}

TEST_F(AuthServiceTest, ValidateTokenDeactivatedUser) {
    // Register, login, then deactivate
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    auto loginResult = authService->login("test@example.com", "Password123");
    authService->deactivateUser(registerResult.userId);
    
    // Try to validate token
    auto result = authService->validateToken(loginResult.token);
    
    EXPECT_FALSE(result.authenticated);
    EXPECT_EQ(result.message, "User account is deactivated");
}

TEST_F(AuthServiceTest, ChangePasswordSuccess) {
    // Register user
    auto registerResult = authService->registerUser("test@example.com", "OldPassword123");
    
    // Change password
    bool success = authService->changePassword(registerResult.userId, "OldPassword123", "NewPassword456");
    EXPECT_TRUE(success);
    
    // Verify old password doesn't work
    auto loginResult1 = authService->login("test@example.com", "OldPassword123");
    EXPECT_FALSE(loginResult1.success);
    
    // Verify new password works
    auto loginResult2 = authService->login("test@example.com", "NewPassword456");
    EXPECT_TRUE(loginResult2.success);
}

TEST_F(AuthServiceTest, ChangePasswordWrongOldPassword) {
    // Register user
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    
    // Try to change with wrong old password
    bool success = authService->changePassword(registerResult.userId, "WrongPassword", "NewPassword456");
    EXPECT_FALSE(success);
}

TEST_F(AuthServiceTest, ChangePasswordWeakNewPassword) {
    // Register user
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    
    // Try to change to weak password
    bool success = authService->changePassword(registerResult.userId, "Password123", "weak");
    EXPECT_FALSE(success);
}

TEST_F(AuthServiceTest, DeactivateAndReactivateUser) {
    // Register user
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    
    // Deactivate
    bool deactivated = authService->deactivateUser(registerResult.userId);
    EXPECT_TRUE(deactivated);
    
    // Verify cannot login
    auto loginResult1 = authService->login("test@example.com", "Password123");
    EXPECT_FALSE(loginResult1.success);
    
    // Reactivate
    bool reactivated = authService->reactivateUser(registerResult.userId);
    EXPECT_TRUE(reactivated);
    
    // Verify can login again
    auto loginResult2 = authService->login("test@example.com", "Password123");
    EXPECT_TRUE(loginResult2.success);
}

TEST_F(AuthServiceTest, GetUserById) {
    // Register user
    auto registerResult = authService->registerUser("test@example.com", "Password123");
    
    // Get user by ID
    auto userOpt = authService->getUserById(registerResult.userId);
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->getEmail(), "test@example.com");
}

TEST_F(AuthServiceTest, GetUserByEmail) {
    // Register user
    authService->registerUser("test@example.com", "Password123");
    
    // Get user by email
    auto userOpt = authService->getUserByEmail("test@example.com");
    ASSERT_TRUE(userOpt.has_value());
    EXPECT_EQ(userOpt->getEmail(), "test@example.com");
}
