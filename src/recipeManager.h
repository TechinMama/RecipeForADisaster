#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <vector>
#include <string>
#include <optional>
#include "recipe.h"

class recipeManager {
public:
    // Custom exception for database operations
    class DatabaseError : public std::runtime_error {
    public:
        explicit DatabaseError(const std::string& message) : std::runtime_error(message) {}
    };

    // Result structure for operations that can fail
    struct OperationResult {
        bool success;
        std::string errorMessage;

        OperationResult(bool s = true, const std::string& msg = "") : success(s), errorMessage(msg) {}
    };

    recipeManager(const std::string& uri);

    OperationResult addRecipe(const recipe& recipe);
    std::vector<recipe> viewRecipes();
    OperationResult updateRecipe(const std::string& title, const recipe& updatedRecipe);
    OperationResult deleteRecipe(const std::string& title);
    std::vector<recipe> searchRecipes(const std::string& criteria);

    // Connection validation
    bool isConnected();

    // Enhanced search methods
    std::vector<recipe> searchByCategory(const std::string& category);
    std::vector<recipe> searchByType(const std::string& type);
    std::vector<recipe> searchByCategoryAndType(const std::string& category, const std::string& type);

    // Pagination support
    struct PaginatedResult {
        std::vector<recipe> recipes;
        int totalCount;
        int page;
        int pageSize;
        int totalPages;
    };

    PaginatedResult getRecipesPaginated(int page = 1, int pageSize = 10);
    PaginatedResult searchRecipesPaginated(const std::string& criteria, int page = 1, int pageSize = 10);

    // JSON response methods for web interface
    std::string getRecipesJson();
    std::string searchRecipesJson(const std::string& criteria);
    std::string addRecipeJson(const std::string& jsonInput);
    std::string updateRecipeJson(const std::string& title, const std::string& jsonInput);
    std::string deleteRecipeJson(const std::string& title);

private:
    mongocxx::client client;
    mongocxx::database db;

    // Helper method to safely extract string from BSON document
    std::string safeGetString(const bsoncxx::document::view& doc, const std::string& fieldName);
};

#endif // RECIPE_MANAGER_H
