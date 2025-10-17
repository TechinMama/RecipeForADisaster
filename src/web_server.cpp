#include <crow.h>
#include <crow/middlewares/cors.h>
#include "recipeManagerSQLite.h"
#include "aiService.h"
#include "vaultService.h"
#include "user.h"
#include "userManager.h"
#include "collection.h"
#include "collectionManager.h"
#include "jwtService.h"
#include "authService.h"
#include "jwtMiddleware.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <filesystem>

// Utility function to get database path from environment variable with fallback
std::string getDatabasePath(const std::string& envVar, const std::string& defaultFilename) {
    const char* envPath = std::getenv(envVar.c_str());
    if (envPath && !std::string(envPath).empty()) {
        return std::string(envPath);
    }

    // Fallback to $HOME/.recipeforadisaster/ directory
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        std::filesystem::path dataDir = std::filesystem::path(homeDir) / ".recipeforadisaster";
        try {
            std::filesystem::create_directories(dataDir);
            return (dataDir / defaultFilename).string();
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Warning: Failed to create data directory " << dataDir << ": " << e.what() << std::endl;
        }
    }

    // Final fallback to current directory
    return defaultFilename;
}

// Custom middleware for error handling
struct ErrorHandler {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {}

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        if (res.code >= 400) {
            res.add_header("Content-Type", "application/json");
        }
    }
};

int main() {
    // Initialize recipe manager
    std::unique_ptr<RecipeManagerSQLite> managerPtr;

    // Use SQLite database
    try {
        std::string recipesDbPath = getDatabasePath("RECIPES_DB_PATH", "recipes.db");
        std::cout << "Using recipes database: " << recipesDbPath << std::endl;
        managerPtr = std::make_unique<RecipeManagerSQLite>(recipesDbPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize SQLite database: " << e.what() << std::endl;
        return 1;
    }

    RecipeManagerSQLite& manager = *managerPtr;    // Check database connection
    if (!manager.isConnected()) {
        std::cerr << "Failed to connect to SQLite database." << std::endl;
        return 1;
    }

    std::cout << "Connected to SQLite database successfully!" << std::endl;

    // Initialize AI service (optional - will be null if not configured)
    std::unique_ptr<AIService> aiService = nullptr;
    std::unique_ptr<VaultService> vaultService = nullptr;

    // Try to initialize Vault service first
    const char* vaultAddr = std::getenv("VAULT_ADDR");
    const char* vaultToken = std::getenv("VAULT_TOKEN");

    if (vaultAddr && vaultToken) {
        try {
            VaultService::VaultConfig vaultConfig;
            vaultConfig.address = vaultAddr;
            vaultConfig.token = vaultToken;
            vaultConfig.mountPath = "secret";

            vaultService = std::make_unique<VaultService>(vaultConfig);
            std::cout << "Connected to Vault successfully!" << std::endl;

            // Try to initialize AI service using Vault credentials
            try {
                aiService = std::make_unique<AIService>(vaultService.get(), "azure-openai");
                if (aiService->isConnected()) {
                    std::cout << "Connected to Azure OpenAI via Vault successfully!" << std::endl;
                } else {
                    std::cout << "Warning: Azure OpenAI service initialized via Vault but connection test failed." << std::endl;
                }
            } catch (const AIService::AIServiceError& e) {
                std::cout << "Warning: Failed to initialize Azure OpenAI service via Vault: " << e.what() << std::endl;
                aiService = nullptr;
            }
        } catch (const std::exception& e) {
            std::cout << "Warning: Failed to initialize Vault service: " << e.what() << std::endl;
            vaultService = nullptr;
        }
    }

    // Fallback to environment variables if Vault initialization failed
    if (!aiService) {
        const char* azureEndpoint = std::getenv("AZURE_OPENAI_ENDPOINT");
        const char* azureApiKey = std::getenv("AZURE_OPENAI_KEY");
        const char* azureDeployment = std::getenv("AZURE_OPENAI_DEPLOYMENT");

        if (azureEndpoint && azureApiKey && azureDeployment) {
            try {
                aiService = std::make_unique<AIService>(azureEndpoint, azureApiKey, azureDeployment);
                if (aiService->isConnected()) {
                    std::cout << "Connected to Azure OpenAI via environment variables successfully!" << std::endl;
                } else {
                    std::cout << "Warning: Azure OpenAI service initialized but connection test failed." << std::endl;
                }
            } catch (const AIService::AIServiceError& e) {
                std::cout << "Warning: Failed to initialize Azure OpenAI service: " << e.what() << std::endl;
                aiService = nullptr;
            }
        } else {
            std::cout << "Azure OpenAI not configured. Recipe generation features will be unavailable." << std::endl;
            if (!vaultService) {
                std::cout << "Set VAULT_ADDR and VAULT_TOKEN to use Vault for credentials, or set AZURE_OPENAI_ENDPOINT, AZURE_OPENAI_KEY, and AZURE_OPENAI_DEPLOYMENT directly." << std::endl;
            } else {
                std::cout << "Vault is configured but Azure OpenAI credentials not found in Vault at path 'secret/azure-openai'." << std::endl;
            }
        }
    }

    // Initialize authentication services
    std::shared_ptr<UserManager> userManager = nullptr;
    std::shared_ptr<JwtService> jwtService = nullptr;
    std::shared_ptr<AuthService> authService = nullptr;
    std::shared_ptr<CollectionManager> collectionManager = nullptr;

    try {
        // Open users database
        std::string usersDbPath = getDatabasePath("USERS_DB_PATH", "users.db");
        std::cout << "Using users database: " << usersDbPath << std::endl;
        sqlite3* usersDb = nullptr;
        int rc = sqlite3_open(usersDbPath.c_str(), &usersDb);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to open users database");
        }

        // Create users table if it doesn't exist
        const char* createTableSql = R"(
            CREATE TABLE IF NOT EXISTS users (
                id TEXT PRIMARY KEY,
                email TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                is_active INTEGER NOT NULL DEFAULT 1,
                name TEXT,
                bio TEXT,
                avatar_url TEXT,
                preferences TEXT,
                privacy_settings TEXT
            )
        )";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(usersDb, createTableSql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::string error = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            throw std::runtime_error("Failed to create users table: " + error);
        }

        userManager = std::make_shared<UserManager>(usersDb);

        // Initialize JWT service
        JwtService::Config jwtConfig;
        
        // Get JWT configuration from environment
        const char* jwtSecret = std::getenv("JWT_SECRET");
        if (jwtSecret) {
            jwtConfig.secret = jwtSecret;
        } else {
            std::cerr << "Warning: JWT_SECRET not set. Using insecure development secret." << std::endl;
            jwtConfig.secret = "change-me-development-secret";
        }

        const char* jwtIssuer = std::getenv("JWT_ISSUER");
        if (jwtIssuer) {
            jwtConfig.issuer = jwtIssuer;
        } else {
            jwtConfig.issuer = "RecipeForADisaster";
        }

        const char* jwtAudience = std::getenv("JWT_AUDIENCE");
        if (jwtAudience) {
            jwtConfig.audience = jwtAudience;
        } else {
            jwtConfig.audience = "RecipeForADisaster-API";
        }

        const char* jwtExpiration = std::getenv("JWT_EXPIRATION_SECONDS");
        if (jwtExpiration) {
            try {
                long seconds = std::stol(jwtExpiration);
                if (seconds > 0) {
                    jwtConfig.accessTokenLifetime = std::chrono::seconds(seconds);
                }
            } catch (...) {
                std::cerr << "Warning: Invalid JWT_EXPIRATION_SECONDS value" << std::endl;
            }
        }

        jwtService = std::make_shared<JwtService>(jwtConfig);
        authService = std::make_shared<AuthService>(userManager, jwtService);

        // Initialize CollectionManager with the same database as UserManager
        std::string recipesDbPath = getDatabasePath("RECIPES_DB_PATH", "recipes.db");
        collectionManager = std::make_shared<CollectionManager>(usersDb, recipesDbPath);

        std::cout << "Authentication and collection services initialized successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize authentication services: " << e.what() << std::endl;
        std::cerr << "Authentication endpoints will be unavailable." << std::endl;
    }

    // Create Crow app with CORS middleware
    crow::App<crow::CORSHandler, ErrorHandler> app;

    // Configure CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global().origin("*").methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method);

    // Helper function to create JSON error response
    auto createErrorResponse = [](const std::string& message, int code = 500) {
        crow::json::wvalue error;
        error["success"] = false;
        error["error"] = message;
        crow::response res(code, error);
        return res;
    };

    // Helper function to create JSON success response
    auto createSuccessResponse = [](crow::json::wvalue data) {
        crow::json::wvalue response;
        response["success"] = true;
        response["data"] = std::move(data);
        crow::response res(200, response);
        return res;
    };

    // GET /api/recipes - Get all recipes
    CROW_ROUTE(app, "/api/recipes")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res) {
        try {
            auto recipes = manager.getAllRecipes();

            crow::json::wvalue data;
            crow::json::wvalue recipes_json = crow::json::wvalue::list();
            for (size_t i = 0; i < recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["title"] = recipes[i].getTitle();
                recipe_json["ingredients"] = recipes[i].getIngredients();
                recipe_json["instructions"] = recipes[i].getInstructions();
                recipe_json["servingSize"] = recipes[i].getServingSize();
                recipe_json["cookTime"] = recipes[i].getCookTime();
                recipe_json["category"] = recipes[i].getCategory();
                recipe_json["type"] = recipes[i].getType();
                recipes_json[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes_json);

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to get recipes: ") + e.what(), 500);
        }
        res.end();
    });

    // GET /api/recipes/search - Search recipes
    CROW_ROUTE(app, "/api/recipes/search")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res) {
        try {
            std::string criteria;

            if (req.url_params.get("q")) {
                criteria = req.url_params.get("q");
            }

            auto recipes = manager.searchByTitle(criteria);

            crow::json::wvalue data;
            crow::json::wvalue recipes_json = crow::json::wvalue::list();
            for (size_t i = 0; i < recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["title"] = recipes[i].getTitle();
                recipe_json["ingredients"] = recipes[i].getIngredients();
                recipe_json["instructions"] = recipes[i].getInstructions();
                recipe_json["servingSize"] = recipes[i].getServingSize();
                recipe_json["cookTime"] = recipes[i].getCookTime();
                recipe_json["category"] = recipes[i].getCategory();
                recipe_json["type"] = recipes[i].getType();
                recipes_json[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes_json);

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to search recipes: ") + e.what(), 500);
        }
        res.end();
    });

    // GET /api/recipes/advanced-search - Advanced search with multiple criteria
    CROW_ROUTE(app, "/api/recipes/advanced-search")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res) {
        try {
            RecipeManagerSQLite::SearchCriteria criteria;
            
            // Extract query parameters
            if (req.url_params.get("q")) {
                criteria.query = req.url_params.get("q");
            }
            if (req.url_params.get("category")) {
                criteria.category = req.url_params.get("category");
            }
            if (req.url_params.get("type")) {
                criteria.type = req.url_params.get("type");
            }
            if (req.url_params.get("cookTimeMax")) {
                criteria.cookTimeMax = req.url_params.get("cookTimeMax");
            }
            if (req.url_params.get("servingSizeMin")) {
                criteria.servingSizeMin = req.url_params.get("servingSizeMin");
            }
            if (req.url_params.get("servingSizeMax")) {
                criteria.servingSizeMax = req.url_params.get("servingSizeMax");
            }
            if (req.url_params.get("ingredient")) {
                criteria.ingredient = req.url_params.get("ingredient");
            }
            if (req.url_params.get("sortBy")) {
                criteria.sortBy = req.url_params.get("sortBy");
            }
            if (req.url_params.get("sortOrder")) {
                criteria.sortOrder = req.url_params.get("sortOrder");
            }

            auto recipes = manager.advancedSearch(criteria);

            crow::json::wvalue data;
            crow::json::wvalue recipes_json = crow::json::wvalue::list();
            for (size_t i = 0; i < recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["id"] = recipes[i].getId();
                recipe_json["title"] = recipes[i].getTitle();
                recipe_json["ingredients"] = recipes[i].getIngredients();
                recipe_json["instructions"] = recipes[i].getInstructions();
                recipe_json["servingSize"] = recipes[i].getServingSize();
                recipe_json["cookTime"] = recipes[i].getCookTime();
                recipe_json["category"] = recipes[i].getCategory();
                recipe_json["type"] = recipes[i].getType();
                recipes_json[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes_json);
            data["count"] = recipes.size();

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to perform advanced search: ") + e.what(), 500);
        }
        res.end();
    });

    // GET /api/health - Health check endpoint
    CROW_ROUTE(app, "/api/health")
    .methods("GET"_method)
    ([&manager](const crow::request& req, crow::response& res) {
        crow::json::wvalue health;
        health["status"] = "healthy";
        health["database"] = manager.isConnected() ? "connected" : "disconnected";
        res.write(health.dump());
        res.code = 200;
        res.end();
    });

    // ==================== AUTHENTICATION ENDPOINTS ====================

    // POST /api/auth/register - Register a new user
    CROW_ROUTE(app, "/api/auth/register")
    .methods("POST"_method)
    ([&authService, &createErrorResponse](const crow::request& req, crow::response& res) {
        if (!authService) {
            res = createErrorResponse("Authentication service not available", 503);
            res.end();
            return;
        }

        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                res = createErrorResponse("Invalid JSON", 400);
                res.end();
                return;
            }

            std::string email = body["email"].s();
            std::string password = body["password"].s();

            if (email.empty() || password.empty()) {
                res = createErrorResponse("Email and password are required", 400);
                res.end();
                return;
            }

            auto result = authService->registerUser(email, password);

            if (result.success) {
                crow::json::wvalue response;
                response["success"] = true;
                response["message"] = result.message;
                response["data"]["userId"] = result.userId;
                res = crow::response(201, response);
            } else {
                res = createErrorResponse(result.message, 400);
            }
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Registration failed: ") + e.what(), 500);
        }
        res.end();
    });

    // POST /api/auth/login - Login user and get JWT token
    CROW_ROUTE(app, "/api/auth/login")
    .methods("POST"_method)
    ([&authService, &createErrorResponse](const crow::request& req, crow::response& res) {
        if (!authService) {
            res = createErrorResponse("Authentication service not available", 503);
            res.end();
            return;
        }

        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                res = createErrorResponse("Invalid JSON", 400);
                res.end();
                return;
            }

            std::string email = body["email"].s();
            std::string password = body["password"].s();

            if (email.empty() || password.empty()) {
                res = createErrorResponse("Email and password are required", 400);
                res.end();
                return;
            }

            auto result = authService->login(email, password);

            if (result.success) {
                crow::json::wvalue response;
                response["success"] = true;
                response["message"] = result.message;
                response["data"]["token"] = result.token;
                response["data"]["userId"] = result.userId;
                response["data"]["email"] = result.email;
                res = crow::response(200, response);
            } else {
                res = createErrorResponse(result.message, 401);
            }
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Login failed: ") + e.what(), 500);
        }
        res.end();
    });

    // POST /api/auth/validate - Validate JWT token
    CROW_ROUTE(app, "/api/auth/validate")
    .methods("POST"_method)
    ([&authService, &createErrorResponse](const crow::request& req, crow::response& res) {
        if (!authService) {
            res = createErrorResponse("Authentication service not available", 503);
            res.end();
            return;
        }

        try {
            // Get token from Authorization header
            std::string authHeader = req.get_header_value("Authorization");
            std::string token;

            if (authHeader.substr(0, 7) == "Bearer ") {
                token = authHeader.substr(7);
            } else {
                // Try to get from body as fallback
                auto body = crow::json::load(req.body);
                if (body && body.has("token")) {
                    token = body["token"].s();
                }
            }

            if (token.empty()) {
                res = createErrorResponse("No token provided", 401);
                res.end();
                return;
            }

            auto result = authService->validateToken(token);

            if (result.authenticated) {
                crow::json::wvalue response;
                response["success"] = true;
                response["message"] = result.message;
                response["data"]["userId"] = result.userId;
                response["data"]["email"] = result.email;
                res = crow::response(200, response);
            } else {
                res = createErrorResponse(result.message, 401);
            }
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Token validation failed: ") + e.what(), 500);
        }
        res.end();
    });

    // GET /api/auth/me - Get current user profile
    CROW_ROUTE(app, "/api/auth/me")
    .methods("GET"_method)
    ([&authService, &createErrorResponse](const crow::request& req, crow::response& res) {
        if (!authService) {
            res = createErrorResponse("Authentication service not available", 503);
            res.end();
            return;
        }

        try {
            // Get token from Authorization header
            std::string authHeader = req.get_header_value("Authorization");
            std::string token;

            if (authHeader.substr(0, 7) == "Bearer ") {
                token = authHeader.substr(7);
            }

            if (token.empty()) {
                res = createErrorResponse("No token provided", 401);
                res.end();
                return;
            }

            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            auto userOpt = authService->getUserById(authResult.userId);
            if (!userOpt.has_value()) {
                res = createErrorResponse("User not found", 404);
                res.end();
                return;
            }

            const auto& user = userOpt.value();
            crow::json::wvalue response;
            response["success"] = true;
            response["data"]["id"] = user.getId();
            response["data"]["email"] = user.getEmail();
            response["data"]["isActive"] = user.isActive();
            response["data"]["name"] = user.getName();
            response["data"]["bio"] = user.getBio();
            response["data"]["avatarUrl"] = user.getAvatarUrl();
            response["data"]["preferences"] = crow::json::load(user.getPreferences());
            response["data"]["privacySettings"] = crow::json::load(user.getPrivacySettings());
            
            res = crow::response(200, response);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to get user profile: ") + e.what(), 500);
        }
        res.end();
    });

    // PUT /api/auth/me - Update current user profile
    CROW_ROUTE(app, "/api/auth/me")
    .methods("PUT"_method)
    ([&authService, &createErrorResponse](const crow::request& req, crow::response& res) {
        if (!authService) {
            res = createErrorResponse("Authentication service not available", 503);
            res.end();
            return;
        }

        try {
            // Get token from Authorization header
            std::string authHeader = req.get_header_value("Authorization");
            std::string token;

            if (authHeader.substr(0, 7) == "Bearer ") {
                token = authHeader.substr(7);
            }

            if (token.empty()) {
                res = createErrorResponse("No token provided", 401);
                res.end();
                return;
            }

            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            auto userOpt = authService->getUserById(authResult.userId);
            if (!userOpt.has_value()) {
                res = createErrorResponse("User not found", 404);
                res.end();
                return;
            }

            auto body = crow::json::load(req.body);
            if (!body) {
                res = createErrorResponse("Invalid JSON", 400);
                res.end();
                return;
            }

            auto& user = userOpt.value();

            // Update profile fields if provided
            if (body.has("name")) {
                user.setName(body["name"].s());
            }
            if (body.has("bio")) {
                user.setBio(body["bio"].s());
            }
            if (body.has("avatarUrl")) {
                user.setAvatarUrl(body["avatarUrl"].s());
            }
            if (body.has("preferences")) {
                user.setPreferences(std::string(body["preferences"]));
            }
            if (body.has("privacySettings")) {
                user.setPrivacySettings(std::string(body["privacySettings"]));
            }

            // Update user in database
            if (authService->updateUser(user)) {
                crow::json::wvalue response;
                response["success"] = true;
                response["message"] = "Profile updated successfully";
                response["data"]["id"] = user.getId();
                response["data"]["email"] = user.getEmail();
                response["data"]["isActive"] = user.isActive();
                response["data"]["name"] = user.getName();
                response["data"]["bio"] = user.getBio();
                response["data"]["avatarUrl"] = user.getAvatarUrl();
                response["data"]["preferences"] = crow::json::load(user.getPreferences());
                response["data"]["privacySettings"] = crow::json::load(user.getPrivacySettings());
                res = crow::response(200, response);
            } else {
                res = createErrorResponse("Failed to update profile", 500);
            }
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to update profile: ") + e.what(), 500);
        }
        res.end();
    });

    // POST /api/auth/change-password - Change user password
    CROW_ROUTE(app, "/api/auth/change-password")
    .methods("POST"_method)
    ([&authService, &createErrorResponse](const crow::request& req, crow::response& res) {
        if (!authService) {
            res = createErrorResponse("Authentication service not available", 503);
            res.end();
            return;
        }

        try {
            // Get token from Authorization header
            std::string authHeader = req.get_header_value("Authorization");
            std::string token;

            if (authHeader.substr(0, 7) == "Bearer ") {
                token = authHeader.substr(7);
            }

            if (token.empty()) {
                res = createErrorResponse("No token provided", 401);
                res.end();
                return;
            }

            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            auto body = crow::json::load(req.body);
            if (!body) {
                res = createErrorResponse("Invalid JSON", 400);
                res.end();
                return;
            }

            std::string oldPassword = body["oldPassword"].s();
            std::string newPassword = body["newPassword"].s();

            if (oldPassword.empty() || newPassword.empty()) {
                res = createErrorResponse("Old password and new password are required", 400);
                res.end();
                return;
            }

            bool success = authService->changePassword(authResult.userId, oldPassword, newPassword);

            if (success) {
                crow::json::wvalue response;
                response["success"] = true;
                response["message"] = "Password changed successfully";
                res = crow::response(200, response);
            } else {
                res = createErrorResponse("Failed to change password", 400);
            }
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Password change failed: ") + e.what(), 500);
        }
        res.end();
    });

    // ==================== END AUTHENTICATION ENDPOINTS ====================

    // GET /api/recipes/categories/<string> - Get recipes by category
    CROW_ROUTE(app, "/api/recipes/categories/<string>")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res, const std::string& category) {
        try {
            auto recipes = manager.searchByCategory(category);

            crow::json::wvalue data;
            crow::json::wvalue recipes_json = crow::json::wvalue::list();
            for (size_t i = 0; i < recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["title"] = recipes[i].getTitle();
                recipe_json["ingredients"] = recipes[i].getIngredients();
                recipe_json["instructions"] = recipes[i].getInstructions();
                recipe_json["servingSize"] = recipes[i].getServingSize();
                recipe_json["cookTime"] = recipes[i].getCookTime();
                recipe_json["category"] = recipes[i].getCategory();
                recipe_json["type"] = recipes[i].getType();
                recipes_json[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes_json);

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to get recipes by category: ") + e.what(), 500);
        }
        res.end();
    });

    // GET /api/recipes/types/<string> - Get recipes by type
    CROW_ROUTE(app, "/api/recipes/types/<string>")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res, const std::string& type) {
        try {
            auto recipes = manager.searchByType(type);

            crow::json::wvalue data;
            crow::json::wvalue recipes_json = crow::json::wvalue::list();
            for (size_t i = 0; i < recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["title"] = recipes[i].getTitle();
                recipe_json["ingredients"] = recipes[i].getIngredients();
                recipe_json["instructions"] = recipes[i].getInstructions();
                recipe_json["servingSize"] = recipes[i].getServingSize();
                recipe_json["cookTime"] = recipes[i].getCookTime();
                recipe_json["category"] = recipes[i].getCategory();
                recipe_json["type"] = recipes[i].getType();
                recipes_json[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes_json);

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to get recipes by type: ") + e.what(), 500);
        }
        res.end();
    });

    // POST /api/recipes - Add a new recipe (PROTECTED - requires authentication)
    CROW_ROUTE(app, "/api/recipes")
    .methods("POST"_method)
    ([&manager, &jwtService, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res) {
        // JWT Authentication - validate token
        auto authResult = JWTMiddleware::validateRequest(req, jwtService);
        if (!authResult.authenticated) {
            res = JWTMiddleware::createAuthErrorResponse(authResult.error);
            res.end();
            return;
        }
        
        try {
            auto json_body = crow::json::load(req.body);

            if (!json_body) {
                res = createErrorResponse("Invalid JSON body", 400);
                res.end();
                return;
            }

            // Extract recipe data from JSON
            std::string title = json_body["title"].s();
            std::string ingredients = json_body["ingredients"].s();
            std::string instructions = json_body["instructions"].s();
            std::string servingSize = json_body["servingSize"].s();
            std::string cookTime = json_body["cookTime"].s();
            std::string category = json_body["category"].s();
            std::string type = json_body["type"].s();

            // Create recipe object
            recipe newRecipe(title, ingredients, instructions, servingSize, cookTime, category, type);

            // Add to database
            bool success = manager.addRecipe(newRecipe);

            if (success) {
                crow::json::wvalue data;
                data["message"] = "Recipe added successfully";
                data["title"] = title;
                data["userId"] = authResult.userId;  // Include userId in response
                res = createSuccessResponse(data);
            } else {
                res = createErrorResponse("Failed to add recipe to database", 500);
            }
        } catch (const recipe::ValidationError& e) {
            res = createErrorResponse(std::string("Validation error: ") + e.what(), 400);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to add recipe: ") + e.what(), 500);
        }
        res.end();
    });

    // PUT /api/recipes/<string> - Update a recipe (PROTECTED - requires authentication)
    CROW_ROUTE(app, "/api/recipes/<string>")
    .methods("PUT"_method)
    ([&manager, &jwtService, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res, const std::string& title) {
        // JWT Authentication - validate token
        auto authResult = JWTMiddleware::validateRequest(req, jwtService);
        if (!authResult.authenticated) {
            res = JWTMiddleware::createAuthErrorResponse(authResult.error);
            res.end();
            return;
        }
        
        try {
            auto json_body = crow::json::load(req.body);

            if (!json_body) {
                res = createErrorResponse("Invalid JSON body", 400);
                res.end();
                return;
            }

            // Extract updated recipe data from JSON
            std::string newTitle = json_body["title"].s();
            std::string ingredients = json_body["ingredients"].s();
            std::string instructions = json_body["instructions"].s();
            std::string servingSize = json_body["servingSize"].s();
            std::string cookTime = json_body["cookTime"].s();
            std::string category = json_body["category"].s();
            std::string type = json_body["type"].s();

            // Create updated recipe object
            recipe updatedRecipe(newTitle, ingredients, instructions, servingSize, cookTime, category, type);

            // Update in database
            bool success = manager.updateRecipe(title, updatedRecipe);

            if (success) {
                crow::json::wvalue data;
                data["message"] = "Recipe updated successfully";
                data["oldTitle"] = title;
                data["newTitle"] = newTitle;
                data["userId"] = authResult.userId;  // Include userId in response
                res = createSuccessResponse(data);
            } else {
                res = createErrorResponse("Recipe not found or update failed", 404);
            }
        } catch (const recipe::ValidationError& e) {
            res = createErrorResponse(std::string("Validation error: ") + e.what(), 400);
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to update recipe: ") + e.what(), 500);
        }
        res.end();
    });

    // DELETE /api/recipes/<string> - Delete a recipe (PROTECTED - requires authentication)
    CROW_ROUTE(app, "/api/recipes/<string>")
    .methods("DELETE"_method)
    ([&manager, &jwtService, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res, const std::string& title) {
        // JWT Authentication - validate token
        auto authResult = JWTMiddleware::validateRequest(req, jwtService);
        if (!authResult.authenticated) {
            res = JWTMiddleware::createAuthErrorResponse(authResult.error);
            res.end();
            return;
        }
        
        try {
            bool success = manager.deleteRecipe(title);

            if (success) {
                crow::json::wvalue data;
                data["message"] = "Recipe deleted successfully";
                data["title"] = title;
                res = createSuccessResponse(data);
            } else {
                res = createErrorResponse("Recipe not found or delete failed", 404);
            }
        } catch (const std::exception& e) {
            res = createErrorResponse(std::string("Failed to delete recipe: ") + e.what(), 500);
        }
        res.end();
    });


    // POST /api/recipes/generate - Generate recipe using AI
    CROW_ROUTE(app, "/api/recipes/generate")
    .methods("POST"_method)
    ([&aiService, &createErrorResponse, &createSuccessResponse](const crow::request& req, crow::response& res) {
        if (!aiService) {
            res = createErrorResponse("AI service not configured. Please set Azure OpenAI environment variables.", 503);
            res.end();
            return;
        }

        try {
            auto json_body = crow::json::load(req.body);
            if (!json_body) {
                res = createErrorResponse("Invalid JSON in request body", 400);
                res.end();
                return;
            }

            if (!json_body.has("prompt")) {
                res = createErrorResponse("Missing 'prompt' field in request body", 400);
                res.end();
                return;
            }

            std::string prompt = json_body["prompt"].s();
            int count = 1;

            if (json_body.has("count")) {
                count = json_body["count"].i();
                if (count < 1 || count > 5) {
                    res = createErrorResponse("Count must be between 1 and 5", 400);
                    res.end();
                    return;
                }
            }

            if (count == 1) {
                // Generate single recipe
                auto result = aiService->generateRecipe(prompt);

                if (!result.success) {
                    res = createErrorResponse(result.errorMessage, 500);
                } else {
                    crow::json::wvalue data;
                    data["generatedRecipe"] = result.generatedContent;
                    data["tokenCount"] = result.tokenCount;
                    res = createSuccessResponse(data);
                }
            } else {
                // Generate multiple recipe suggestions
                auto results = aiService->generateRecipeSuggestions(prompt, count);

                crow::json::wvalue data;
                crow::json::wvalue suggestions = crow::json::wvalue::list();

                for (size_t i = 0; i < results.size(); ++i) {
                    crow::json::wvalue suggestion;
                    suggestion["success"] = results[i].success;
                    if (results[i].success) {
                        suggestion["content"] = results[i].generatedContent;
                        suggestion["tokenCount"] = results[i].tokenCount;
                    } else {
                        suggestion["error"] = results[i].errorMessage;
                    }
                    suggestions[i] = std::move(suggestion);
                }

                data["suggestions"] = std::move(suggestions);
                res = createSuccessResponse(data);
            }

        } catch (const std::exception& e) {
            res = createErrorResponse("Unexpected error: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // ==========================================
    // COLLECTION ENDPOINTS
    // ==========================================

    // GET /api/collections - Get user's collections
    CROW_ROUTE(app, "/api/collections")
    .methods("GET"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // Get user's collections
            auto collections = collectionManager->getUserCollections(userId);

            crow::json::wvalue data;
            data["collections"] = crow::json::wvalue::list();

            for (const auto& collection : collections) {
                crow::json::wvalue collectionJson;
                collectionJson["id"] = collection.getId();
                collectionJson["name"] = collection.getName();
                collectionJson["description"] = collection.getDescription();
                collectionJson["userId"] = collection.getUserId();
                collectionJson["privacySettings"] = collection.getPrivacySettings();
                collectionJson["createdAt"] = collection.getCreatedAt();
                collectionJson["updatedAt"] = collection.getUpdatedAt();

                // Get recipe count for this collection
                int recipeCount = collectionManager->getCollectionRecipeCount(collection.getId());
                collectionJson["recipeCount"] = recipeCount;

                data["collections"][data["collections"].size()] = std::move(collectionJson);
            }

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to get collections: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // POST /api/collections - Create new collection
    CROW_ROUTE(app, "/api/collections")
    .methods("POST"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // Parse JSON body
            crow::json::rvalue json_body;
            try {
                json_body = crow::json::load(req.body);
            } catch (const std::exception& e) {
                res = createErrorResponse("Invalid JSON in request body", 400);
                res.end();
                return;
            }

            // Validate required fields
            if (!json_body.has("name") || !json_body["name"].s().size()) {
                res = createErrorResponse("Collection name is required", 400);
                res.end();
                return;
            }

            std::string name = json_body["name"].s();
            std::string description = json_body.has("description") ? std::string(json_body["description"].s()) : "";
            std::string privacySettings = json_body.has("privacySettings") ? std::string(json_body["privacySettings"].s()) : "private";

            // Validate privacy settings
            if (privacySettings != "private" && privacySettings != "public" && privacySettings != "shared") {
                res = createErrorResponse("Invalid privacy settings. Must be 'private', 'public', or 'shared'", 400);
                res.end();
                return;
            }

            // Generate ID for the collection
            std::string collectionId = User::generateId();

            // Create collection
            Collection newCollection(name, description, userId, privacySettings, collectionId);
            bool success = collectionManager->createCollection(newCollection);

            if (!success) {
                res = createErrorResponse("Failed to create collection", 500);
                res.end();
                return;
            }

            // Get the created collection
            auto createdCollection = collectionManager->getCollectionById(collectionId);
            if (!createdCollection) {
                res = createErrorResponse("Collection created but could not retrieve details", 500);
                res.end();
                return;
            }

            crow::json::wvalue data;
            data["collection"]["id"] = createdCollection->getId();
            data["collection"]["name"] = createdCollection->getName();
            data["collection"]["description"] = createdCollection->getDescription();
            data["collection"]["userId"] = createdCollection->getUserId();
            data["collection"]["privacySettings"] = createdCollection->getPrivacySettings();
            data["collection"]["createdAt"] = createdCollection->getCreatedAt();
            data["collection"]["updatedAt"] = createdCollection->getUpdatedAt();
            data["collection"]["recipeCount"] = 0;

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to create collection: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // GET /api/collections/<id> - Get specific collection
    CROW_ROUTE(app, "/api/collections/<string>")
    .methods("GET"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res, std::string collectionIdStr) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // Collection ID is already a string from the URL parameter
            std::string collectionId = collectionIdStr;

            // Get collection
            auto collection = collectionManager->getCollectionById(collectionId);
            if (!collection) {
                res = createErrorResponse("Collection not found", 404);
                res.end();
                return;
            }

            // Check if user owns this collection or if it's public
            if (collection->getUserId() != userId && collection->getPrivacySettings() == "private") {
                res = createErrorResponse("Access denied", 403);
                res.end();
                return;
            }

            crow::json::wvalue data;
            data["collection"]["id"] = collection->getId();
            data["collection"]["name"] = collection->getName();
            data["collection"]["description"] = collection->getDescription();
            data["collection"]["userId"] = collection->getUserId();
            data["collection"]["privacySettings"] = collection->getPrivacySettings();
            data["collection"]["createdAt"] = collection->getCreatedAt();
            data["collection"]["updatedAt"] = collection->getUpdatedAt();

            // Get recipes in this collection (just IDs for now)
            auto recipeIds = collectionManager->getRecipeIdsInCollection(collectionId);
            data["collection"]["recipes"] = crow::json::wvalue::list();

            for (const auto& recipeId : recipeIds) {
                crow::json::wvalue recipeJson;
                recipeJson["id"] = recipeId;
                // TODO: Add full recipe details when recipe model is updated
                data["collection"]["recipes"][data["collection"]["recipes"].size()] = std::move(recipeJson);
            }

            data["collection"]["recipeCount"] = recipeIds.size();

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to get collection: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // PUT /api/collections/<id> - Update collection
    CROW_ROUTE(app, "/api/collections/<string>")
    .methods("PUT"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res, std::string collectionIdStr) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // Collection ID is already a string from the URL parameter
            std::string collectionId = collectionIdStr;

            // Check if collection exists and user owns it
            auto existingCollection = collectionManager->getCollectionById(collectionId);
            if (!existingCollection) {
                res = createErrorResponse("Collection not found", 404);
                res.end();
                return;
            }

            if (existingCollection->getUserId() != userId) {
                res = createErrorResponse("Access denied", 403);
                res.end();
                return;
            }

            // Parse JSON body
            crow::json::rvalue json_body;
            try {
                json_body = crow::json::load(req.body);
            } catch (const std::exception& e) {
                res = createErrorResponse("Invalid JSON in request body", 400);
                res.end();
                return;
            }

            // Update collection fields
            std::string name = json_body.has("name") ? std::string(json_body["name"].s()) : existingCollection->getName();
            std::string description = json_body.has("description") ? std::string(json_body["description"].s()) : existingCollection->getDescription();
            std::string privacySettings = json_body.has("privacySettings") ? std::string(json_body["privacySettings"].s()) : existingCollection->getPrivacySettings();

            // Validate required fields
            if (name.empty()) {
                res = createErrorResponse("Collection name cannot be empty", 400);
                res.end();
                return;
            }

            // Validate privacy settings
            if (privacySettings != "private" && privacySettings != "public" && privacySettings != "shared") {
                res = createErrorResponse("Invalid privacy settings. Must be 'private', 'public', or 'shared'", 400);
                res.end();
                return;
            }

            // Update collection
            Collection updatedCollection(collectionId, name, description, userId, privacySettings);
            bool success = collectionManager->updateCollection(updatedCollection);

            if (!success) {
                res = createErrorResponse("Failed to update collection", 500);
                res.end();
                return;
            }

            // Get updated collection
            auto finalCollection = collectionManager->getCollectionById(collectionId);
            if (!finalCollection) {
                res = createErrorResponse("Collection updated but could not retrieve details", 500);
                res.end();
                return;
            }

            crow::json::wvalue data;
            data["collection"]["id"] = finalCollection->getId();
            data["collection"]["name"] = finalCollection->getName();
            data["collection"]["description"] = finalCollection->getDescription();
            data["collection"]["userId"] = finalCollection->getUserId();
            data["collection"]["privacySettings"] = finalCollection->getPrivacySettings();
            data["collection"]["createdAt"] = finalCollection->getCreatedAt();
            data["collection"]["updatedAt"] = finalCollection->getUpdatedAt();

            // Get recipe count
            int recipeCount = collectionManager->getCollectionRecipeCount(collectionId);
            data["collection"]["recipeCount"] = recipeCount;

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to update collection: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // DELETE /api/collections/<id> - Delete collection
    CROW_ROUTE(app, "/api/collections/<string>")
    .methods("DELETE"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res, std::string collectionIdStr) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // Collection ID is already a string from the URL parameter
            std::string collectionId = collectionIdStr;

            // Check if collection exists and user owns it
            auto existingCollection = collectionManager->getCollectionById(collectionId);
            if (!existingCollection) {
                res = createErrorResponse("Collection not found", 404);
                res.end();
                return;
            }

            if (existingCollection->getUserId() != userId) {
                res = createErrorResponse("Access denied", 403);
                res.end();
                return;
            }

            // Delete collection
            bool success = collectionManager->deleteCollection(collectionId);

            if (!success) {
                res = createErrorResponse("Failed to delete collection", 500);
                res.end();
                return;
            }

            crow::json::wvalue data;
            data["message"] = "Collection deleted successfully";

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to delete collection: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // POST /api/collections/<id>/recipes/<recipeId> - Add recipe to collection
    CROW_ROUTE(app, "/api/collections/<string>/recipes/<string>")
    .methods("POST"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res, std::string collectionIdStr, std::string recipeIdStr) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // IDs are already strings from URL parameters
            std::string collectionId = collectionIdStr;
            std::string recipeId = recipeIdStr;

            // Check if collection exists and user owns it
            auto collection = collectionManager->getCollectionById(collectionId);
            if (!collection) {
                res = createErrorResponse("Collection not found", 404);
                res.end();
                return;
            }

            if (collection->getUserId() != userId) {
                res = createErrorResponse("Access denied", 403);
                res.end();
                return;
            }

            // Check if recipe exists (we'll assume it does for now, but in production you'd validate)
            // Add recipe to collection
            bool success = collectionManager->addRecipeToCollection(collectionId, recipeId);

            if (!success) {
                res = createErrorResponse("Failed to add recipe to collection", 500);
                res.end();
                return;
            }

            crow::json::wvalue data;
            data["message"] = "Recipe added to collection successfully";
            data["collectionId"] = collectionId;
            data["recipeId"] = recipeId;

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to add recipe to collection: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // DELETE /api/collections/<id>/recipes/<recipeId> - Remove recipe from collection
    CROW_ROUTE(app, "/api/collections/<string>/recipes/<string>")
    .methods("DELETE"_method)
    ([&collectionManager, &authService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res, std::string collectionIdStr, std::string recipeIdStr) {
        try {
            // Extract and validate JWT token
            auto authHeader = req.get_header_value("Authorization");
            if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
                res = createErrorResponse("Missing or invalid authorization header", 401);
                res.end();
                return;
            }

            std::string token = authHeader.substr(7);
            auto authResult = authService->validateToken(token);
            if (!authResult.authenticated) {
                res = createErrorResponse(authResult.message, 401);
                res.end();
                return;
            }

            std::string userId = authResult.userId;

            // IDs are already strings from URL parameters
            std::string collectionId = collectionIdStr;
            std::string recipeId = recipeIdStr;

            // Check if collection exists and user owns it
            auto collection = collectionManager->getCollectionById(collectionId);
            if (!collection) {
                res = createErrorResponse("Collection not found", 404);
                res.end();
                return;
            }

            if (collection->getUserId() != userId) {
                res = createErrorResponse("Access denied", 403);
                res.end();
                return;
            }

            // Remove recipe from collection
            bool success = collectionManager->removeRecipeFromCollection(collectionId, recipeId);

            if (!success) {
                res = createErrorResponse("Failed to remove recipe from collection", 500);
                res.end();
                return;
            }

            crow::json::wvalue data;
            data["message"] = "Recipe removed from collection successfully";
            data["collectionId"] = collectionId;
            data["recipeId"] = recipeId;

            res = createSuccessResponse(data);
        } catch (const std::exception& e) {
            res = createErrorResponse("Failed to remove recipe from collection: " + std::string(e.what()), 500);
        }
        res.end();
    });

    // GET /api/ai/status - Check AI service status
    CROW_ROUTE(app, "/api/ai/status")
    .methods("GET"_method)
    ([&aiService, &createSuccessResponse, &createErrorResponse](const crow::request& req, crow::response& res) {
        crow::json::wvalue data;
        data["aiServiceConfigured"] = (aiService != nullptr);

        if (aiService) {
            data["aiServiceConnected"] = aiService->isConnected();
        } else {
            data["aiServiceConnected"] = false;
            data["configurationHelp"] = "Set AZURE_OPENAI_ENDPOINT, AZURE_OPENAI_KEY, and AZURE_OPENAI_DEPLOYMENT environment variables";
        }

        res = createSuccessResponse(data);
        res.end();
    });

    // Serve static files (for frontend)
    CROW_ROUTE(app, "/")
    ([]() {
        std::string html = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RecipeForADisaster - AI-Powered Recipe Manager</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        .header {
            text-align: center;
            color: white;
            margin-bottom: 30px;
        }
        .header h1 {
            font-size: 3em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .card {
            background: white;
            border-radius: 12px;
            padding: 25px;
            margin-bottom: 20px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            border: 1px solid rgba(255,255,255,0.2);
        }
        .ai-section {
            background: linear-gradient(135deg, #ff9a9e 0%, #fecfef 100%);
            color: #333;
        }
        .ai-form {
            display: flex;
            gap: 10px;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        .ai-input {
            flex: 1;
            min-width: 200px;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        .ai-input:focus {
            outline: none;
            border-color: #667eea;
        }
        .ai-button {
            padding: 12px 24px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 16px;
            font-weight: bold;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        .ai-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(0,0,0,0.2);
        }
        .ai-button:disabled {
            background: #ccc;
            cursor: not-allowed;
            transform: none;
        }
        .result {
            background: #f8f9fa;
            border-left: 4px solid #667eea;
            padding: 15px;
            margin-top: 15px;
            border-radius: 4px;
            white-space: pre-wrap;
            font-family: 'Courier New', monospace;
            max-height: 400px;
            overflow-y: auto;
        }
        .error {
            background: #f8d7da;
            border-left: 4px solid #dc3545;
            color: #721c24;
        }
        .loading {
            display: inline-block;
            width: 20px;
            height: 20px;
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            animation: spin 1s linear infinite;
            margin-right: 10px;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        .api-section {
            background: linear-gradient(135deg, #a8edea 0%, #fed6e3 100%);
        }
        .api-list {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 15px;
        }
        .api-item {
            background: rgba(255,255,255,0.9);
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        .status-online { background: #28a745; }
        .status-offline { background: #dc3545; }
        .footer {
            text-align: center;
            color: white;
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid rgba(255,255,255,0.2);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🍳 RecipeForADisaster</h1>
            <p>AI-Powered Recipe Manager</p>
        </div>

        <div class="card ai-section">
            <h2>🤖 AI Recipe Generation</h2>
            <div class="ai-form">
                <input type="text" id="aiPrompt" class="ai-input" placeholder="Describe your recipe idea... (e.g., 'Italian pasta with vegetables')" maxlength="500">
                <select id="suggestionCount" class="ai-input" style="max-width: 150px;">
                    <option value="1">1 Recipe</option>
                    <option value="2">2 Recipes</option>
                    <option value="3" selected>3 Recipes</option>
                    <option value="4">4 Recipes</option>
                    <option value="5">5 Recipes</option>
                </select>
                <button id="generateBtn" class="ai-button" onclick="generateRecipe()">Generate Recipe</button>
            </div>
            <div id="aiResult" style="display: none;"></div>
        </div>

        <div class="card api-section">
            <h2>📡 API Endpoints</h2>
            <div class="api-list">
                <div class="api-item">
                    <strong>GET /api/recipes</strong><br>
                    Get all recipes with pagination
                </div>
                <div class="api-item">
                    <strong>POST /api/recipes/generate</strong><br>
                    Generate recipes using AI
                </div>
                <div class="api-item">
                    <strong>GET /api/recipes/search?q=query</strong><br>
                    Search recipes by title
                </div>
                <div class="api-item">
                    <strong>POST /api/recipes</strong><br>
                    Add new recipe manually
                </div>
                <div class="api-item">
                    <strong>GET /api/ai/status</strong><br>
                    Check AI service status
                </div>
                <div class="api-item">
                    <strong>GET /api/health</strong><br>
                    System health check
                </div>
            </div>
        </div>

        <div class="card">
            <h2>🔍 Search Existing Recipes</h2>
            <div class="ai-form">
                <input type="text" id="searchQuery" class="ai-input" placeholder="Search recipes...">
                <button class="ai-button" onclick="searchRecipes()">Search</button>
            </div>
            <div id="searchResult" style="display: none;"></div>
        </div>
    </div>

    <div class="footer">
        <p>Made with ❤️ for learning C++, databases, and AI integration</p>
    </div>

    <script>
        async function generateRecipe() {
            const prompt = document.getElementById('aiPrompt').value.trim();
            const count = document.getElementById('suggestionCount').value;
            const resultDiv = document.getElementById('aiResult');
            const generateBtn = document.getElementById('generateBtn');

            if (!prompt) {
                showResult('Please enter a recipe description.', true);
                return;
            }

            generateBtn.disabled = true;
            generateBtn.innerHTML = '<span class="loading"></span>Generating...';

            try {
                const response = await fetch('/api/recipes/generate', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ prompt: prompt, count: parseInt(count) })
                });

                const data = await response.json();

                if (data.success) {
                    if (count == 1) {
                        showResult(data.data.generatedRecipe, false);
                    } else {
                        let result = '';
                        data.data.suggestions.forEach((suggestion, index) => {
                            result += `=== Recipe ${index + 1} ===\n`;
                            if (suggestion.success) {
                                result += suggestion.content + '\n\n';
                            } else {
                                result += `Error: ${suggestion.error}\n\n`;
                            }
                        });
                        showResult(result, false);
                    }
                } else {
                    showResult(data.error, true);
                }
            } catch (error) {
                showResult('Network error: ' + error.message, true);
            } finally {
                generateBtn.disabled = false;
                generateBtn.innerHTML = 'Generate Recipe';
            }
        }

        async function searchRecipes() {
            const query = document.getElementById('searchQuery').value.trim();
            const resultDiv = document.getElementById('searchResult');

            if (!query) {
                showSearchResult('Please enter a search term.', true);
                return;
            }

            try {
                const response = await fetch(`/api/recipes/search?q=${encodeURIComponent(query)}`);
                const data = await response.json();

                if (data.success && data.data.recipes.length > 0) {
                    let result = `Found ${data.data.recipes.length} recipe(s):\n\n`;
                    data.data.recipes.forEach(recipe => {
                        result += `**${recipe.title}**\n`;
                        result += `Category: ${recipe.category} | Type: ${recipe.type}\n`;
                        result += `Cook Time: ${recipe.cookTime} | Servings: ${recipe.servingSize}\n\n`;
                    });
                    showSearchResult(result, false);
                } else {
                    showSearchResult('No recipes found matching your search.', false);
                }
            } catch (error) {
                showSearchResult('Search error: ' + error.message, true);
            }
        }

        function showResult(content, isError) {
            const resultDiv = document.getElementById('aiResult');
            resultDiv.style.display = 'block';
            resultDiv.className = isError ? 'result error' : 'result';
            resultDiv.textContent = content;
        }

        function showSearchResult(content, isError) {
            const resultDiv = document.getElementById('searchResult');
            resultDiv.style.display = 'block';
            resultDiv.className = isError ? 'result error' : 'result';
            resultDiv.textContent = content;
        }

        // Check AI status on page load
        window.onload = async function() {
            try {
                const response = await fetch('/api/ai/status');
                const data = await response.json();
                const indicator = document.createElement('div');
                indicator.className = 'status-indicator ' + (data.success && data.data.aiServiceConnected ? 'status-online' : 'status-offline');
                indicator.title = data.success && data.data.aiServiceConnected ? 'AI Service Online' : 'AI Service Offline';

                const header = document.querySelector('.ai-section h2');
                header.insertBefore(indicator, header.firstChild);
            } catch (e) {
                console.log('Could not check AI status');
            }
        };

        // Allow Enter key to trigger generation
        document.getElementById('aiPrompt').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                generateRecipe();
            }
        });

        document.getElementById('searchQuery').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                searchRecipes();
            }
        });
    </script>
</body>
</html>
)html";
        return crow::response(200, html);
    });

    // Start the server
    std::cout << "Starting web server on http://localhost:8080" << std::endl;
    std::cout << "API endpoints:" << std::endl;
    std::cout << "  GET  /api/recipes - Get all recipes" << std::endl;
    std::cout << "  GET  /api/recipes/search?q=query - Search recipes" << std::endl;
    std::cout << "  GET  /api/recipes/categories/category - Get recipes by category" << std::endl;
    std::cout << "  GET  /api/recipes/types/type - Get recipes by type" << std::endl;
    std::cout << "  POST /api/recipes - Add new recipe" << std::endl;
    std::cout << "  PUT  /api/recipes/title - Update recipe" << std::endl;
    std::cout << "  DELETE /api/recipes/title - Delete recipe" << std::endl;
    std::cout << "  POST /api/recipes/generate - Generate recipe with AI" << std::endl;
    std::cout << "  GET  /api/ai/status - Check AI service status" << std::endl;
    std::cout << "  GET  /api/health - Health check" << std::endl;
    std::cout << "Web interface: http://localhost:8080" << std::endl;

    app.port(8080).multithreaded().run();

    return 0;
}