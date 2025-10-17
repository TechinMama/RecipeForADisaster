#include "user.h"
#include <regex>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <random>
#include <chrono>

User::User()
    : id_(""), email_(""), password_hash_(""),
      created_at_(std::chrono::system_clock::now()),
      updated_at_(std::chrono::system_clock::now()),
      is_active_(true), name_(""), bio_(""), avatar_url_(""),
      preferences_(nlohmann::json::object()), privacy_settings_(nlohmann::json::object()) {
}

User::User(const std::string& email, const std::string& password)
    : id_(generateId()), email_(email), password_hash_(hashPassword(password)),
      created_at_(std::chrono::system_clock::now()),
      updated_at_(std::chrono::system_clock::now()),
      is_active_(true), name_(""), bio_(""), avatar_url_(""),
      preferences_(nlohmann::json::object()), privacy_settings_(nlohmann::json::object()) {
}

User::User(const std::string& id, const std::string& email, const std::string& passwordHash,
           const std::chrono::system_clock::time_point& createdAt,
           const std::chrono::system_clock::time_point& updatedAt,
           bool isActive, const std::string& name, const std::string& bio,
           const std::string& avatarUrl, const nlohmann::json& preferences,
           const nlohmann::json& privacySettings)
    : id_(id), email_(email), password_hash_(passwordHash),
      created_at_(createdAt), updated_at_(updatedAt), is_active_(isActive),
      name_(name), bio_(bio), avatar_url_(avatarUrl),
      preferences_(preferences), privacy_settings_(privacySettings) {
}

bool User::validateEmail() const {
    // Basic email validation regex
    const std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email_, emailRegex);
}

bool User::validatePassword(const std::string& password) const {
    // Password requirements: at least 8 characters, 1 uppercase, 1 lowercase, 1 digit
    if (password.length() < 8) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false;
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
    }

    return hasUpper && hasLower && hasDigit;
}

bool User::verifyPassword(const std::string& password) const {
    // Hash the provided password and compare with stored hash
    return hashPassword(password) == password_hash_;
}

std::string User::hashPassword(const std::string& password) const {
    // Simple SHA-256 hashing using modern EVP API (in production, use proper password hashing like bcrypt)
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    const EVP_MD* md = EVP_sha256();
    if (EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize digest");
    }

    if (EVP_DigestUpdate(ctx, password.c_str(), password.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to update digest");
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize digest");
    }

    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

nlohmann::json User::toJson() const {
    return {
        {"id", id_},
        {"email", email_},
        {"password_hash", password_hash_},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(
            created_at_.time_since_epoch()).count()},
        {"updated_at", std::chrono::duration_cast<std::chrono::seconds>(
            updated_at_.time_since_epoch()).count()},
        {"is_active", is_active_},
        {"name", name_},
        {"bio", bio_},
        {"avatar_url", avatar_url_},
        {"preferences", preferences_},
        {"privacy_settings", privacy_settings_}
    };
}

User User::fromJson(const nlohmann::json& json) {
    auto createdAt = std::chrono::system_clock::time_point(
        std::chrono::seconds(json["created_at"].get<long long>()));
    auto updatedAt = std::chrono::system_clock::time_point(
        std::chrono::seconds(json["updated_at"].get<long long>()));

    // Handle optional profile fields with defaults
    std::string name = json.value("name", "");
    std::string bio = json.value("bio", "");
    std::string avatarUrl = json.value("avatar_url", "");
    nlohmann::json preferences = json.value("preferences", nlohmann::json::object());
    nlohmann::json privacySettings = json.value("privacy_settings", nlohmann::json::object());

    return User(
        json["id"].get<std::string>(),
        json["email"].get<std::string>(),
        json["password_hash"].get<std::string>(),
        createdAt,
        updatedAt,
        json["is_active"].get<bool>(),
        name,
        bio,
        avatarUrl,
        preferences,
        privacySettings
    );
}

std::string User::generateId() {
    // Generate a simple UUID-like string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) ss << "-";
        ss << dis(gen);
    }
    return ss.str();
}