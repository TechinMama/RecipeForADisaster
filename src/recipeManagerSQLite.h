#ifndef RECIPE_MANAGER_SQLITE_H
#define RECIPE_MANAGER_SQLITE_H

#include <string>
#include <vector>
#include <memory>
#include "recipe.h"

// SQLite-based recipe manager (alternative to MongoDB)
class RecipeManagerSQLite {
public:
    explicit RecipeManagerSQLite(const std::string& dbPath = "recipes.db");
    ~RecipeManagerSQLite();

    // Core CRUD operations
    bool addRecipe(const recipe& recipe);
    bool updateRecipe(const std::string& id, const recipe& recipe);
    bool deleteRecipe(const std::string& id);
    std::unique_ptr<recipe> getRecipe(const std::string& id);
    std::vector<recipe> getAllRecipes();

    // Search operations
    std::vector<recipe> searchByTitle(const std::string& title);
    std::vector<recipe> searchByCategory(const std::string& category);
    std::vector<recipe> searchByType(const std::string& type);

    // Utility
    bool isConnected() const;
    void initializeDatabase();

private:
    std::string dbPath_;
    void* db_; // sqlite3* (avoid including sqlite3.h in header)

    // Helper methods
    std::string generateId();
    std::string recipeToJson(const recipe& recipe);
    recipe jsonToRecipe(const std::string& json);
};

#endif // RECIPE_MANAGER_SQLITE_H