#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

class User {
public:
    User();
    User(const std::string& email, const std::string& password);
    User(const std::string& id, const std::string& email, const std::string& passwordHash,
         const std::chrono::system_clock::time_point& createdAt,
         const std::chrono::system_clock::time_point& updatedAt,
         bool isActive);

    // Getters
    std::string getId() const { return id_; }
    std::string getEmail() const { return email_; }
    std::string getPasswordHash() const { return password_hash_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return created_at_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updated_at_; }
    bool isActive() const { return is_active_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& hash) { password_hash_ = hash; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& time) { updated_at_ = time; }
    void setActive(bool active) { is_active_ = active; }

    // Utility methods
    bool validateEmail() const;
    bool validatePassword(const std::string& password) const;
    bool verifyPassword(const std::string& password) const;
    std::string hashPassword(const std::string& password) const;

    // Serialization
    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& json);

    // ID generation
    static std::string generateId();

private:
    std::string id_;
    std::string email_;
    std::string password_hash_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
    bool is_active_;
};