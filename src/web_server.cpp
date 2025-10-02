#include <crow.h>
#include <crow/middlewares/cors.h>
#include "recipeManager.h"
#include "aiService.h"
#include "vaultService.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

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
    // Initialize MongoDB instance
    mongocxx::instance instance{};

    // Initialize Vault service (optional - fallback to environment variables if not configured)
    std::unique_ptr<VaultService> vaultService = nullptr;
    const char* vaultAddress = std::getenv("VAULT_ADDR");
    const char* vaultToken = std::getenv("VAULT_TOKEN");

    if (vaultAddress && vaultToken) {
        try {
            VaultService::VaultConfig vaultConfig;
            vaultConfig.address = vaultAddress;
            vaultConfig.token = vaultToken;

            vaultService = std::make_unique<VaultService>(vaultConfig);
            std::cout << "Vault service initialized successfully!" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Warning: Failed to initialize Vault service: " << e.what() << std::endl;
            std::cout << "Falling back to environment variable configuration." << std::endl;
            vaultService = nullptr;
        }
    } else {
        std::cout << "Vault not configured. Using environment variables for credentials." << std::endl;
        std::cout << "Set VAULT_ADDR and VAULT_TOKEN to enable Vault-based credential management." << std::endl;
    }

    // Initialize recipe manager
    std::unique_ptr<recipeManager> managerPtr;

    if (vaultService) {
        // Use Vault for secure credential retrieval
        try {
            managerPtr = std::make_unique<recipeManager>(vaultService.get(), "database/mongodb");
        } catch (const recipeManager::DatabaseError& e) {
            std::cerr << "Failed to initialize database with Vault credentials: " << e.what() << std::endl;
            return 1;
        }
    } else {
        // Fallback to environment variables
        const char* mongoUri = std::getenv("MONGODB_URI");
        if (!mongoUri) {
            mongoUri = "mongodb://localhost:27017";
        }

        try {
            managerPtr = std::make_unique<recipeManager>(mongoUri);
        } catch (const recipeManager::DatabaseError& e) {
            std::cerr << "Failed to connect to MongoDB. Please check your connection string." << std::endl;
            return 1;
        }
    }

    recipeManager& manager = *managerPtr;

    // Check database connection
    if (!manager.isConnected()) {
        std::cerr << "Failed to connect to MongoDB." << std::endl;
        return 1;
    }

    std::cout << "Connected to MongoDB successfully!" << std::endl;

    // Initialize AI service (optional - will be null if not configured)
    std::unique_ptr<AIService> aiService = nullptr;

    if (vaultService) {
        // Use Vault for secure credential retrieval
        try {
            aiService = std::make_unique<AIService>(vaultService.get(), "ai/azure-openai");
            if (aiService->isConnected()) {
                std::cout << "Connected to Azure OpenAI successfully!" << std::endl;
            } else {
                std::cout << "Warning: Azure OpenAI service initialized but connection test failed." << std::endl;
            }
        } catch (const AIService::AIServiceError& e) {
            std::cout << "Warning: Failed to initialize Azure OpenAI service with Vault: " << e.what() << std::endl;
            aiService = nullptr;
        }
    } else {
        // Fallback to environment variables
        const char* azureEndpoint = std::getenv("AZURE_OPENAI_ENDPOINT");
        const char* azureApiKey = std::getenv("AZURE_OPENAI_KEY");
        const char* azureDeployment = std::getenv("AZURE_OPENAI_DEPLOYMENT");

        if (azureEndpoint && azureApiKey && azureDeployment) {
            try {
                aiService = std::make_unique<AIService>(azureEndpoint, azureApiKey, azureDeployment);
                if (aiService->isConnected()) {
                    std::cout << "Connected to Azure OpenAI successfully!" << std::endl;
                } else {
                    std::cout << "Warning: Azure OpenAI service initialized but connection test failed." << std::endl;
                }
            } catch (const AIService::AIServiceError& e) {
                std::cout << "Warning: Failed to initialize Azure OpenAI service: " << e.what() << std::endl;
                aiService = nullptr;
            }
        } else {
            std::cout << "Azure OpenAI not configured. Recipe generation features will be unavailable." << std::endl;
            std::cout << "Set AZURE_OPENAI_ENDPOINT, AZURE_OPENAI_KEY, and AZURE_OPENAI_DEPLOYMENT to enable AI features." << std::endl;
        }
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

    // GET /api/recipes - Get all recipes with pagination
    CROW_ROUTE(app, "/api/recipes")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req) {
        try {
            int page = 1;
            int pageSize = 10;

            // Parse query parameters
            if (req.url_params.get("page")) {
                page = std::stoi(req.url_params.get("page"));
            }
            if (req.url_params.get("pageSize")) {
                pageSize = std::stoi(req.url_params.get("pageSize"));
            }

            auto result = manager.getRecipesPaginated(page, pageSize);

            crow::json::wvalue data;
            data["page"] = result.page;
            data["pageSize"] = result.pageSize;
            data["totalCount"] = result.totalCount;
            data["totalPages"] = result.totalPages;

            crow::json::wvalue recipes = crow::json::wvalue::list();
            for (size_t i = 0; i < result.recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["title"] = result.recipes[i].getTitle();
                recipe_json["ingredients"] = result.recipes[i].getIngredients();
                recipe_json["instructions"] = result.recipes[i].getInstructions();
                recipe_json["servingSize"] = result.recipes[i].getServingSize();
                recipe_json["cookTime"] = result.recipes[i].getCookTime();
                recipe_json["category"] = result.recipes[i].getCategory();
                recipe_json["type"] = result.recipes[i].getType();
                recipes[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes);

            return createSuccessResponse(data);
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to get recipes: ") + e.what(), 500);
        }
    });

    // GET /api/recipes/search - Search recipes with pagination
    CROW_ROUTE(app, "/api/recipes/search")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req) {
        try {
            std::string criteria;
            int page = 1;
            int pageSize = 10;

            if (req.url_params.get("q")) {
                criteria = req.url_params.get("q");
            }
            if (req.url_params.get("page")) {
                page = std::stoi(req.url_params.get("page"));
            }
            if (req.url_params.get("pageSize")) {
                pageSize = std::stoi(req.url_params.get("pageSize"));
            }

            auto result = manager.searchRecipesPaginated(criteria, page, pageSize);

            crow::json::wvalue data;
            data["page"] = result.page;
            data["pageSize"] = result.pageSize;
            data["totalCount"] = result.totalCount;
            data["totalPages"] = result.totalPages;

            crow::json::wvalue recipes = crow::json::wvalue::list();
            for (size_t i = 0; i < result.recipes.size(); ++i) {
                crow::json::wvalue recipe_json;
                recipe_json["title"] = result.recipes[i].getTitle();
                recipe_json["ingredients"] = result.recipes[i].getIngredients();
                recipe_json["instructions"] = result.recipes[i].getInstructions();
                recipe_json["servingSize"] = result.recipes[i].getServingSize();
                recipe_json["cookTime"] = result.recipes[i].getCookTime();
                recipe_json["category"] = result.recipes[i].getCategory();
                recipe_json["type"] = result.recipes[i].getType();
                recipes[i] = std::move(recipe_json);
            }
            data["recipes"] = std::move(recipes);

            return createSuccessResponse(data);
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to search recipes: ") + e.what(), 500);
        }
    });

    // GET /api/recipes/categories - Get recipes by category
    CROW_ROUTE(app, "/api/recipes/categories/<string>")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, const std::string& category) {
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

            return createSuccessResponse(data);
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to get recipes by category: ") + e.what(), 500);
        }
    });

    // GET /api/recipes/types/<string> - Get recipes by type
    CROW_ROUTE(app, "/api/recipes/types/<string>")
    .methods("GET"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, const std::string& type) {
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

            return createSuccessResponse(data);
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to get recipes by type: ") + e.what(), 500);
        }
    });

    // POST /api/recipes - Add a new recipe
    CROW_ROUTE(app, "/api/recipes")
    .methods("POST"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req) {
        try {
            auto json_body = crow::json::load(req.body);

            if (!json_body) {
                return createErrorResponse("Invalid JSON body", 400);
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
            auto result = manager.addRecipe(newRecipe);

            if (result.success) {
                crow::json::wvalue data;
                data["message"] = "Recipe added successfully";
                data["title"] = title;
                return createSuccessResponse(data);
            } else {
                return createErrorResponse(result.errorMessage, 400);
            }
        } catch (const recipe::ValidationError& e) {
            return createErrorResponse(std::string("Validation error: ") + e.what(), 400);
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to add recipe: ") + e.what(), 500);
        }
    });

    // PUT /api/recipes/<string> - Update a recipe
    CROW_ROUTE(app, "/api/recipes/<string>")
    .methods("PUT"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, const std::string& title) {
        try {
            auto json_body = crow::json::load(req.body);

            if (!json_body) {
                return createErrorResponse("Invalid JSON body", 400);
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
            auto result = manager.updateRecipe(title, updatedRecipe);

            if (result.success) {
                crow::json::wvalue data;
                data["message"] = "Recipe updated successfully";
                data["oldTitle"] = title;
                data["newTitle"] = newTitle;
                return createSuccessResponse(data);
            } else {
                return createErrorResponse(result.errorMessage, 404);
            }
        } catch (const recipe::ValidationError& e) {
            return createErrorResponse(std::string("Validation error: ") + e.what(), 400);
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to update recipe: ") + e.what(), 500);
        }
    });

    // DELETE /api/recipes/<string> - Delete a recipe
    CROW_ROUTE(app, "/api/recipes/<string>")
    .methods("DELETE"_method)
    ([&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req, const std::string& title) {
        try {
            auto result = manager.deleteRecipe(title);

            if (result.success) {
                crow::json::wvalue data;
                data["message"] = "Recipe deleted successfully";
                data["title"] = title;
                return createSuccessResponse(data);
            } else {
                return createErrorResponse(result.errorMessage, 404);
            }
        } catch (const std::exception& e) {
            return createErrorResponse(std::string("Failed to delete recipe: ") + e.what(), 500);
        }
    });

    // GET /api/health - Health check endpoint
    CROW_ROUTE(app, "/api/health")
    ([&manager, &createErrorResponse, &createSuccessResponse]() {
        crow::json::wvalue data;
        data["status"] = "healthy";
        data["database_connected"] = manager.isConnected();
        data["timestamp"] = std::time(nullptr);
        return createSuccessResponse(data);
    });

    // POST /api/recipes/generate - Generate recipe using AI
    CROW_ROUTE(app, "/api/recipes/generate")
    .methods("POST"_method)
    ([&aiService, &createErrorResponse, &createSuccessResponse](const crow::request& req) {
        if (!aiService) {
            return createErrorResponse("AI service not configured. Please set Azure OpenAI environment variables.", 503);
        }

        try {
            auto json_body = crow::json::load(req.body);
            if (!json_body) {
                return createErrorResponse("Invalid JSON in request body", 400);
            }

            if (!json_body.has("prompt")) {
                return createErrorResponse("Missing 'prompt' field in request body", 400);
            }

            std::string prompt = json_body["prompt"].s();
            int count = 1;

            if (json_body.has("count")) {
                count = json_body["count"].i();
                if (count < 1 || count > 5) {
                    return createErrorResponse("Count must be between 1 and 5", 400);
                }
            }

            if (count == 1) {
                // Generate single recipe
                auto result = aiService->generateRecipe(prompt);

                if (!result.success) {
                    return createErrorResponse(result.errorMessage, 500);
                }

                crow::json::wvalue data;
                data["generatedRecipe"] = result.generatedContent;
                data["tokenCount"] = result.tokenCount;
                return createSuccessResponse(data);
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
                return createSuccessResponse(data);
            }

        } catch (const std::exception& e) {
            return createErrorResponse("Unexpected error: " + std::string(e.what()), 500);
        }
    });

    // GET /api/ai/status - Check AI service status
    CROW_ROUTE(app, "/api/ai/status")
    .methods("GET"_method)
    ([&aiService, &createSuccessResponse, &createErrorResponse]() {
        crow::json::wvalue data;
        data["aiServiceConfigured"] = (aiService != nullptr);

        if (aiService) {
            data["aiServiceConnected"] = aiService->isConnected();
        } else {
            data["aiServiceConnected"] = false;
            data["configurationHelp"] = "Set AZURE_OPENAI_ENDPOINT, AZURE_OPENAI_KEY, and AZURE_OPENAI_DEPLOYMENT environment variables";
        }

        return createSuccessResponse(data);
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