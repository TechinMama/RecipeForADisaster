#include "jwtMiddleware.h"
#include <nlohmann/json.hpp>

namespace JWTMiddleware {

std::string extractBearerToken(const std::string& authHeader) {
    const std::string bearerPrefix = "Bearer ";
    
    if (authHeader.length() > bearerPrefix.length() && 
        authHeader.substr(0, bearerPrefix.length()) == bearerPrefix) {
        return authHeader.substr(bearerPrefix.length());
    }
    
    return "";
}

AuthResult validateRequest(const crow::request& req, std::shared_ptr<JwtService> jwtService) {
    AuthResult result;
    
    // Get Authorization header
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.empty()) {
        result.error = "No token provided";
        return result;
    }
    
    // Extract Bearer token
    std::string token = extractBearerToken(authHeader);
    if (token.empty()) {
        result.error = "Invalid Authorization header format. Expected 'Bearer <token>'";
        return result;
    }
    
    // Validate token and extract claims
    try {
        auto claims = jwtService->validateToken(token);
        
        if (!claims.has_value()) {
            result.error = "Invalid or expired token";
            return result;
        }
        
        // Token is valid
        result.authenticated = true;
        result.userId = claims->subject;  // subject contains the user ID
        result.email = claims->email;
        
    } catch (const std::exception& e) {
        result.error = std::string("Token validation failed: ") + e.what();
        return result;
    }
    
    return result;
}

crow::response createAuthErrorResponse(const std::string& message, int statusCode) {
    crow::json::wvalue error;
    error["success"] = false;
    error["error"] = message;
    
    crow::response res(statusCode, error);
    res.add_header("Content-Type", "application/json");
    return res;
}

} // namespace JWTMiddleware
