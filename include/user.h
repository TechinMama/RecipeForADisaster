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
         bool isActive, const std::string& name = "", const std::string& bio = "",
         const std::string& avatarUrl = "", const nlohmann::json& preferences = nlohmann::json::object(),
         const nlohmann::json& privacySettings = nlohmann::json::object());

    // Getters
    std::string getId() const { return id_; }
    std::string getEmail() const { return email_; }
    std::string getPasswordHash() const { return password_hash_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return created_at_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updated_at_; }
    bool isActive() const { return is_active_; }
    std::string getName() const { return name_; }
    std::string getBio() const { return bio_; }
    std::string getAvatarUrl() const { return avatar_url_; }
    nlohmann::json getPreferences() const { return preferences_; }
    nlohmann::json getPrivacySettings() const { return privacy_settings_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& hash) { password_hash_ = hash; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& time) { updated_at_ = time; }
    void setActive(bool active) { is_active_ = active; }
    void setName(const std::string& name) { name_ = name; }
    void setBio(const std::string& bio) { bio_ = bio; }
    void setAvatarUrl(const std::string& url) { avatar_url_ = url; }
    void setPreferences(const nlohmann::json& prefs) { preferences_ = prefs; }
    void setPrivacySettings(const nlohmann::json& settings) { privacy_settings_ = settings; }

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
    std::string name_;
    std::string bio_;
    std::string avatar_url_;
    nlohmann::json preferences_;
    nlohmann::json privacy_settings_;
};