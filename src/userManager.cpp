#include "userManager.h"
#include <iostream>
#include <sstream>

UserManager::UserManager(sqlite3* db) : db_(db) {
    if (!db_) {
        throw std::runtime_error("Database connection is null");
    }
}

bool UserManager::createUser(const User& user) {
    const std::string query = R"(
        INSERT INTO users (id, email, password_hash, created_at, updated_at, is_active, name, bio, avatar_url, preferences, privacy_settings)
        VALUES (?, ?, ?, datetime('now'), datetime('now'), 1, ?, ?, ?, ?, ?)
    )";

    std::vector<std::string> params = {
        user.getId(),
        user.getEmail(),
        user.getPasswordHash(),
        user.getName(),
        user.getBio(),
        user.getAvatarUrl(),
        user.getPreferences().dump(),
        user.getPrivacySettings().dump()
    };

    return executeQuery(query, params);
}

std::optional<User> UserManager::findUserById(const std::string& id) {
    const std::string query = R"(
        SELECT id, email, password_hash, created_at, updated_at, is_active, name, bio, avatar_url, preferences, privacy_settings
        FROM users
        WHERE id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }

    rc = sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    std::optional<User> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = userFromRow(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> UserManager::findUserByEmail(const std::string& email) {
    const std::string query = R"(
        SELECT id, email, password_hash, created_at, updated_at, is_active, name, bio, avatar_url, preferences, privacy_settings
        FROM users
        WHERE email = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }

    rc = sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    std::optional<User> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = userFromRow(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool UserManager::updateUser(const User& user) {
    const std::string query = R"(
        UPDATE users
        SET email = ?, password_hash = ?, updated_at = datetime('now'), is_active = ?,
            name = ?, bio = ?, avatar_url = ?, preferences = ?, privacy_settings = ?
        WHERE id = ?
    )";

    std::vector<std::string> params = {
        user.getEmail(),
        user.getPasswordHash(),
        user.isActive() ? "1" : "0",
        user.getName(),
        user.getBio(),
        user.getAvatarUrl(),
        user.getPreferences().dump(),
        user.getPrivacySettings().dump(),
        user.getId()
    };

    return executeQuery(query, params);
}

bool UserManager::deleteUser(const std::string& id) {
    const std::string query = "DELETE FROM users WHERE id = ?";
    std::vector<std::string> params = {id};
    return executeQuery(query, params);
}

std::vector<User> UserManager::getAllUsers() {
    const std::string query = R"(
        SELECT id, email, password_hash, created_at, updated_at, is_active,
               name, bio, avatar_url, preferences, privacy_settings
        FROM users
        ORDER BY created_at DESC
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    std::vector<User> users;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = userFromRow(stmt);
        if (user) {
            users.push_back(*user);
        }
    }

    sqlite3_finalize(stmt);
    return users;
}

bool UserManager::emailExists(const std::string& email) {
    const std::string query = "SELECT COUNT(*) FROM users WHERE email = ?";
    std::vector<std::string> params = {email};

    auto result = executeScalarQuery(query, params);
    if (result) {
        return std::stoi(*result) > 0;
    }
    return false;
}

bool UserManager::isUserActive(const std::string& id) {
    const std::string query = "SELECT is_active FROM users WHERE id = ?";
    std::vector<std::string> params = {id};

    auto result = executeScalarQuery(query, params);
    if (result) {
        return *result == "1";
    }
    return false;
}

bool UserManager::activateUser(const std::string& id) {
    const std::string query = "UPDATE users SET is_active = 1, updated_at = datetime('now') WHERE id = ?";
    std::vector<std::string> params = {id};
    return executeQuery(query, params);
}

bool UserManager::deactivateUser(const std::string& id) {
    const std::string query = "UPDATE users SET is_active = 0, updated_at = datetime('now') WHERE id = ?";
    std::vector<std::string> params = {id};
    return executeQuery(query, params);
}

// Helper methods
std::optional<User> UserManager::userFromRow(sqlite3_stmt* stmt) const {
    if (!stmt) {
        return std::nullopt;
    }

    try {
        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        // For now, use current time for timestamps (SQLite stores them as TEXT)
        auto now = std::chrono::system_clock::now();
        bool isActive = sqlite3_column_int(stmt, 5) != 0;

        // Read profile fields
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        std::string bio = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        std::string avatarUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        std::string preferencesStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        std::string privacySettingsStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));

        // Parse JSON fields
        nlohmann::json preferences = nlohmann::json::parse(preferencesStr);
        nlohmann::json privacySettings = nlohmann::json::parse(privacySettingsStr);

        // Use constructor with all fields
        User user(id, email, passwordHash, now, now, isActive, name, bio, avatarUrl, preferences, privacySettings);

        return user;
    } catch (const std::exception& e) {
        std::cerr << "Error creating user from row: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool UserManager::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to bind parameter " << (i + 1) << ": " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    return true;
}

std::optional<std::string> UserManager::executeScalarQuery(const std::string& query, const std::vector<std::string>& params) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to bind parameter " << (i + 1) << ": " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return std::nullopt;
        }
    }

    std::optional<std::string> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (text) {
            result = std::string(text);
        }
    }

    sqlite3_finalize(stmt);
    return result;
}