#ifndef JWT_MIDDLEWARE_H
#define JWT_MIDDLEWARE_H

#include <string>
#include <memory>
#include "crow.h"
#include "jwtService.h"

/**
 * JWT Authentication Middleware for Crow Framework
 * 
 * Validates JWT tokens in the Authorization header and extracts user information.
 * Protected routes require a valid Bearer token to access.
 * 
 * Usage:
 *   app.register_blueprint(
 *     crow::Blueprint("protected_api")
 *       .prefix("/api/protected")
 *       .middlewares<JWTAuthMiddleware>()
 *   );
 * 
 * Or apply to individual routes:
 *   CROW_ROUTE(app, "/api/protected/endpoint")
 *     .CROW_MIDDLEWARES(app, JWTAuthMiddleware)
 *     ([](const crow::request& req) { ... });
 */
struct JWTAuthMiddleware {
    /**
     * Context stored during request processing
     * Contains authentication state and user information
     */
    struct context {
        std::string userId;      // User ID from JWT token
        std::string email;       // User email from JWT token
        bool authenticated = false;  // Whether the request is authenticated
        std::string error;       // Error message if authentication failed
    };

    /**
     * Constructor
     * @param jwtService Shared pointer to JWT service for token validation
     */
    JWTAuthMiddleware(std::shared_ptr<JwtService> jwtService);

    /**
     * Called before the route handler
     * Validates JWT token and populates context with user information
     * 
     * @param req HTTP request
     * @param res HTTP response (used to send error if authentication fails)
     * @param ctx Middleware context to store authentication state
     */
    void before_handle(crow::request& req, crow::response& res, context& ctx);

    /**
     * Called after the route handler
     * Can be used for cleanup or logging
     * 
     * @param req HTTP request
     * @param res HTTP response
     * @param ctx Middleware context with authentication state
     */
    void after_handle(crow::request& req, crow::response& res, context& ctx);

private:
    std::shared_ptr<JwtService> jwtService_;

    /**
     * Extract Bearer token from Authorization header
     * 
     * @param authHeader Authorization header value
     * @return Token string without "Bearer " prefix, or empty string if invalid
     */
    std::string extractBearerToken(const std::string& authHeader) const;

    /**
     * Create JSON error response
     * 
     * @param message Error message
     * @param statusCode HTTP status code
     * @return Crow response with JSON error
     */
    crow::response createErrorResponse(const std::string& message, int statusCode) const;
};

#endif // JWT_MIDDLEWARE_H
