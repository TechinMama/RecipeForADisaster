#ifndef JWT_MIDDLEWARE_H
#define JWT_MIDDLEWARE_H

#include <string>
#include <memory>
#include "crow.h"
#include "jwtService.h"

/**
 * JWT Authentication Helper Functions for Crow Framework
 * 
 * Provides helper functions to validate JWT tokens and protect routes.
 * Use these functions at the beginning of protected route handlers.
 * 
 * Example usage:
 *   CROW_ROUTE(app, "/api/recipes").methods("POST"_method)
 *   ([&](const crow::request& req, crow::response& res) {
 *       auto authResult = JWTMiddleware::validateRequest(req, jwtService);
 *       if (!authResult.authenticated) {
 *           res = JWTMiddleware::createAuthErrorResponse(authResult.error);
 *           res.end();
 *           return;
 *       }
 *       // User is authenticated, authResult.userId and authResult.email available
 *       // ... handle the request
 *   });
 */
namespace JWTMiddleware {

    /**
     * Authentication result structure
     */
    struct AuthResult {
        bool authenticated = false;
        std::string userId;
        std::string email;
        std::string error;
    };

    /**
     * Extract Bearer token from Authorization header
     * 
     * @param authHeader Authorization header value (e.g., "Bearer eyJhbG...")
     * @return Token string without "Bearer " prefix, or empty string if invalid
     */
    std::string extractBearerToken(const std::string& authHeader);

    /**
     * Validate JWT token and extract user information
     * 
     * @param req HTTP request
     * @param jwtService JWT service for token validation
     * @return AuthResult with authentication status and user info
     */
    AuthResult validateRequest(const crow::request& req, std::shared_ptr<JwtService> jwtService);

    /**
     * Create JSON error response for authentication failures
     * 
     * @param message Error message
     * @param statusCode HTTP status code (default: 401)
     * @return Crow response with JSON error
     */
    crow::response createAuthErrorResponse(const std::string& message, int statusCode = 401);
}

#endif // JWT_MIDDLEWARE_H
