#include <crow.h>
#include <crow/middlewares/cors.h>
#include "recipeManager.h"
#include <iostream>
#include <string>
#include <vector>

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

    // Initialize recipe manager
    const char* mongoUri = std::getenv("MONGODB_URI");
    if (!mongoUri) {
        mongoUri = "mongodb://localhost:27017";
    }

    recipeManager manager(mongoUri);

    // Check database connection
    if (!manager.isConnected()) {
        std::cerr << "Failed to connect to MongoDB. Please check your connection string." << std::endl;
        return 1;
    }

    std::cout << "Connected to MongoDB successfully!" << std::endl;

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

    // Serve static files (for frontend)
    CROW_ROUTE(app, "/")
    ([]() {
        std::string html = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RecipeForADisaster - Web Interface</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🍳 RecipeForADisaster</h1>
        <p>Web interface coming soon! Use the API endpoints:</p>
        <ul>
            <li>GET /api/recipes - Get all recipes</li>
            <li>POST /api/recipes - Add a recipe</li>
            <li>GET /api/health - Health check</li>
        </ul>
    </div>
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
    std::cout << "  GET  /api/health - Health check" << std::endl;
    std::cout << "Web interface: http://localhost:8080" << std::endl;

    app.port(8080).multithreaded().run();

    return 0;
}