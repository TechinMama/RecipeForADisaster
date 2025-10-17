#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>

class Collection {
public:
    // Custom exception for validation errors
    class ValidationError : public std::runtime_error {
    public:
        explicit ValidationError(const std::string& message) : std::runtime_error(message) {}
    };

    Collection(const std::string& name, const std::string& description,
               const std::string& userId, const std::string& privacySettings = "{}",
               const std::string& id = "", const std::string& createdAt = "",
               const std::string& updatedAt = "");

    std::string getId() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getUserId() const;
    std::string getPrivacySettings() const;
    nlohmann::json getPrivacySettingsJson() const;
    std::string getCreatedAt() const;
    std::string getUpdatedAt() const;

    void setName(const std::string& name);
    void setDescription(const std::string& description);
    void setPrivacySettings(const std::string& privacySettings);

    // JSON serialization methods
    std::string toJson() const;
    static Collection fromJson(const std::string& jsonStr);

private:
    std::string id;
    std::string name;
    std::string description;
    std::string userId;
    std::string privacySettings; // JSON string for privacy settings
    std::string createdAt;
    std::string updatedAt;

    // Validation helper methods
    void validateName(const std::string& name) const;
    void validateDescription(const std::string& description) const;
    void validateUserId(const std::string& userId) const;
    void validatePrivacySettings(const std::string& privacySettings) const;

    // JSON helper methods
    std::string escapeJsonString(const std::string& str) const;
    static std::string unescapeJsonString(const std::string& str);
};

#endif // COLLECTION_H