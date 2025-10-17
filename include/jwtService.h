#pragma once

#include "user.h"
#include <chrono>
#include <optional>
#include <string>

class JwtService {
public:
    struct Config {
        std::string secret;
        std::string issuer = "RecipeForADisaster";
        std::string audience = "RecipeForADisasterClients";
        std::chrono::seconds accessTokenLifetime{3600};
    };

    struct Claims {
        std::string subject; // User ID
        std::string email;
        std::string issuer;
        std::string audience;
        std::chrono::system_clock::time_point issuedAt;
        std::chrono::system_clock::time_point expiresAt;
    };

    explicit JwtService(Config config);

    std::string generateToken(const User& user) const;
    std::optional<Claims> validateToken(const std::string& token) const;

    const Config& getConfig() const { return config_; }

    static Config loadConfigFromEnvironment();

private:
    Config config_;

    static std::string readEnvOrDefault(const char* key, const std::string& defaultValue = "");
    static std::chrono::seconds parseDurationOrDefault(const char* key, std::chrono::seconds defaultValue);
};
