#include "jwtService.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stdexcept>
#include <jwt-cpp/jwt.h>

namespace {
template <typename JsonTraits>
std::string requireClaim(const jwt::decoded_jwt<JsonTraits>& decoded, const std::string& key) {
    if (!decoded.has_payload_claim(key)) {
        throw std::runtime_error("Missing claim: " + key);
    }
    return decoded.get_payload_claim(key).as_string();
}
}

JwtService::JwtService(Config config) : config_(std::move(config)) {
    if (config_.secret.empty()) {
        throw std::invalid_argument("JWT secret must not be empty");
    }
    if (config_.accessTokenLifetime.count() <= 0) {
        throw std::invalid_argument("JWT access token lifetime must be positive");
    }
}

std::string JwtService::generateToken(const User& user) const {
    if (user.getId().empty()) {
        throw std::invalid_argument("User ID must not be empty when generating JWT");
    }
    if (!user.isActive()) {
        throw std::invalid_argument("Inactive users cannot receive JWT tokens");
    }

    const auto now = std::chrono::system_clock::now();
    const auto expiresAt = now + config_.accessTokenLifetime;

    auto token = jwt::create()
        .set_type("JWS")
        .set_issuer(config_.issuer)
        .set_audience(config_.audience)
        .set_subject(user.getId())
        .set_issued_at(now)
        .set_not_before(now)
        .set_expires_at(expiresAt)
        .set_payload_claim("email", jwt::claim(std::string(user.getEmail())))
        .sign(jwt::algorithm::hs256{config_.secret});

    return token;
}

std::optional<JwtService::Claims> JwtService::validateToken(const std::string& token) const {
    if (token.empty()) {
        return std::nullopt;
    }

    try {
        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{config_.secret})
            .with_issuer(config_.issuer);

        if (!config_.audience.empty()) {
            verifier.with_audience(config_.audience);
        }

        verifier.verify(decoded);

        Claims claims{};
        claims.subject = decoded.get_subject();
        claims.email = requireClaim(decoded, "email");
        claims.issuer = decoded.get_issuer();
        const auto& audiences = decoded.get_audience();
        if (!audiences.empty()) {
            claims.audience = *audiences.begin();
        }
        claims.issuedAt = decoded.get_issued_at();
        claims.expiresAt = decoded.get_expires_at();

        return claims;
    } catch (const std::exception& ex) {
        std::cerr << "JWT validation failed: " << ex.what() << std::endl;
        return std::nullopt;
    }
}

JwtService::Config JwtService::loadConfigFromEnvironment() {
    Config config;
    config.secret = readEnvOrDefault("JWT_SECRET");
    if (config.secret.empty()) {
        std::cerr << "Warning: JWT_SECRET not set. Using insecure development secret. Set JWT_SECRET for production." << std::endl;
        config.secret = "change-me-development-secret";
    }

    auto issuer = readEnvOrDefault("JWT_ISSUER");
    if (!issuer.empty()) {
        config.issuer = issuer;
    }

    auto audience = readEnvOrDefault("JWT_AUDIENCE");
    if (!audience.empty()) {
        config.audience = audience;
    }

    config.accessTokenLifetime = parseDurationOrDefault("JWT_EXPIRATION_SECONDS", config.accessTokenLifetime);

    return config;
}

std::string JwtService::readEnvOrDefault(const char* key, const std::string& defaultValue) {
    if (const char* value = std::getenv(key)) {
        return {value};
    }
    return defaultValue;
}

std::chrono::seconds JwtService::parseDurationOrDefault(const char* key, std::chrono::seconds defaultValue) {
    if (const char* value = std::getenv(key)) {
        try {
            long seconds = std::stol(value);
            if (seconds > 0) {
                return std::chrono::seconds(seconds);
            }
        } catch (const std::exception& ex) {
            std::cerr << "Invalid value for " << key << ": " << ex.what() << std::endl;
        }
    }
    return defaultValue;
}
