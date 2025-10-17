#include "collection.h"
#include <algorithm>
#include <cctype>
#include <iostream>

Collection::Collection(const std::string& name, const std::string& description,
                       const std::string& userId, const std::string& privacySettings,
                       const std::string& id, const std::string& createdAt,
                       const std::string& updatedAt)
{
    // Validate all input parameters
    validateName(name);
    validateDescription(description);
    validateUserId(userId);
    validatePrivacySettings(privacySettings);

    // Assign validated values
    this->name = name;
    this->description = description;
    this->userId = userId;
    this->privacySettings = privacySettings;
    this->id = id;
    this->createdAt = createdAt;
    this->updatedAt = updatedAt;
}

std::string Collection::getId() const { return id; }
std::string Collection::getName() const { return name; }
std::string Collection::getDescription() const { return description; }
std::string Collection::getUserId() const { return userId; }
std::string Collection::getPrivacySettings() const { return privacySettings; }
std::string Collection::getCreatedAt() const { return createdAt; }
std::string Collection::getUpdatedAt() const { return updatedAt; }

nlohmann::json Collection::getPrivacySettingsJson() const {
    try {
        return nlohmann::json::parse(privacySettings);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing privacy settings JSON: " << e.what() << std::endl;
        return nlohmann::json::object();
    }
}

void Collection::setName(const std::string& name) {
    validateName(name);
    this->name = name;
}

void Collection::setDescription(const std::string& description) {
    validateDescription(description);
    this->description = description;
}

void Collection::setPrivacySettings(const std::string& privacySettings) {
    validatePrivacySettings(privacySettings);
    this->privacySettings = privacySettings;
}

// Validation methods
void Collection::validateName(const std::string& name) const {
    if (name.empty()) {
        throw ValidationError("Collection name cannot be empty");
    }
    if (name.length() > 100) {
        throw ValidationError("Collection name cannot exceed 100 characters");
    }
}

void Collection::validateDescription(const std::string& description) const {
    if (description.length() > 500) {
        throw ValidationError("Collection description cannot exceed 500 characters");
    }
}

void Collection::validateUserId(const std::string& userId) const {
    if (userId.empty()) {
        throw ValidationError("User ID cannot be empty");
    }
}

void Collection::validatePrivacySettings(const std::string& privacySettings) const {
    if (privacySettings.empty()) {
        return; // Empty string is valid (defaults to public)
    }

    try {
        nlohmann::json::parse(privacySettings);
    } catch (const std::exception& e) {
        throw ValidationError("Privacy settings must be valid JSON");
    }
}

// JSON serialization methods
std::string Collection::toJson() const {
    return "{"
           "\"id\":\"" + escapeJsonString(id) + "\","
           "\"name\":\"" + escapeJsonString(name) + "\","
           "\"description\":\"" + escapeJsonString(description) + "\","
           "\"userId\":\"" + escapeJsonString(userId) + "\","
           "\"privacySettings\":" + privacySettings +
           "}";
}

Collection Collection::fromJson(const std::string& jsonStr) {
    std::string cleanedJson = jsonStr;

    // Remove whitespace
    cleanedJson.erase(std::remove_if(cleanedJson.begin(), cleanedJson.end(),
        [](char c) { return std::isspace(c); }), cleanedJson.end());

    // Basic JSON validation
    if (cleanedJson.empty() || cleanedJson[0] != '{' || cleanedJson.back() != '}') {
        throw ValidationError("Invalid JSON format");
    }

    // Extract field values (simple parsing - for production, consider using a proper JSON library)
    std::string id, name, description, userId, privacySettings = "{}";

    // Helper function to extract string value
    auto extractString = [](const std::string& json, const std::string& field) -> std::string {
        std::string searchStr = "\"" + field + "\":\"";
        size_t startPos = json.find(searchStr);
        if (startPos == std::string::npos) return "";

        startPos += searchStr.length();
        size_t endPos = json.find("\"", startPos);
        if (endPos == std::string::npos) return "";

        std::string value = json.substr(startPos, endPos - startPos);
        return unescapeJsonString(value);
    };

    // Helper function to extract JSON object value
    auto extractJsonObject = [](const std::string& json, const std::string& field) -> std::string {
        std::string searchStr = "\"" + field + "\":";
        size_t startPos = json.find(searchStr);
        if (startPos == std::string::npos) return "{}";

        startPos += searchStr.length();
        size_t braceCount = 0;
        size_t endPos = startPos;

        for (; endPos < json.length(); ++endPos) {
            if (json[endPos] == '{') {
                braceCount++;
            } else if (json[endPos] == '}') {
                braceCount--;
                if (braceCount == 0) {
                    endPos++;
                    break;
                }
            } else if (json[endPos] == ',' && braceCount == 0) {
                break;
            }
        }

        return json.substr(startPos, endPos - startPos);
    };

    id = extractString(cleanedJson, "id");
    name = extractString(cleanedJson, "name");
    description = extractString(cleanedJson, "description");
    userId = extractString(cleanedJson, "userId");
    privacySettings = extractJsonObject(cleanedJson, "privacySettings");

    return Collection(name, description, userId, privacySettings, id);
}

std::string Collection::escapeJsonString(const std::string& str) const {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string Collection::unescapeJsonString(const std::string& str) {
    std::string unescaped;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': unescaped += '"'; ++i; break;
                case '\\': unescaped += '\\'; ++i; break;
                case 'n': unescaped += '\n'; ++i; break;
                case 'r': unescaped += '\r'; ++i; break;
                case 't': unescaped += '\t'; ++i; break;
                default: unescaped += str[i]; break;
            }
        } else {
            unescaped += str[i];
        }
    }
    return unescaped;
}