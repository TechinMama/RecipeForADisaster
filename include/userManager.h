#pragma once

#include "user.h"
#include <sqlite3.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class UserManager {
public:
    explicit UserManager(sqlite3* db);
    ~UserManager() = default;

    // User CRUD operations
    bool createUser(const User& user);
    std::optional<User> findUserById(const std::string& id);
    std::optional<User> findUserByEmail(const std::string& email);
    bool updateUser(const User& user);
    bool deleteUser(const std::string& id);

    // User queries
    std::vector<User> getAllUsers();
    bool emailExists(const std::string& email);
    bool isUserActive(const std::string& id);

    // User status operations
    bool activateUser(const std::string& id);
    bool deactivateUser(const std::string& id);

private:
    sqlite3* db_;

    // Helper methods
    std::optional<User> userFromRow(sqlite3_stmt* stmt) const;
    bool executeQuery(const std::string& query, const std::vector<std::string>& params = {});
    std::optional<std::string> executeScalarQuery(const std::string& query, const std::vector<std::string>& params = {});
};