#include "recipeManagerSQLite.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <nlohmann/json.hpp>

// SQLite-based recipe manager implementation
RecipeManagerSQLite::RecipeManagerSQLite(const std::string& dbPath)
    : dbPath_(dbPath), db_(nullptr) {
    initializeDatabase();
}

RecipeManagerSQLite::~RecipeManagerSQLite() {
    if (db_) {
        sqlite3_close(static_cast<sqlite3*>(db_));
    }
}

void RecipeManagerSQLite::initializeDatabase() {
    int rc = sqlite3_open(dbPath_.c_str(), reinterpret_cast<sqlite3**>(&db_));
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(static_cast<sqlite3*>(db_))));
    }

    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS recipes ("
        "id TEXT PRIMARY KEY,"
        "data BLOB NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create table: " + error);
    }
}

bool RecipeManagerSQLite::isConnected() const {
    return db_ != nullptr;
}

std::string RecipeManagerSQLite::generateId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::stringstream ss;
    ss << "recipe_" << timestamp << "_" << rand() % 1000;
    return ss.str();
}

std::string RecipeManagerSQLite::recipeToJson(const recipe& recipe) {
    nlohmann::json j;
    j["id"] = recipe.getId();
    j["title"] = recipe.getTitle();
    j["ingredients"] = recipe.getIngredients();
    j["instructions"] = recipe.getInstructions();
    j["servingSize"] = recipe.getServingSize();
    j["cookTime"] = recipe.getCookTime();
    j["category"] = recipe.getCategory();
    j["type"] = recipe.getType();

    return j.dump();
}

recipe RecipeManagerSQLite::jsonToRecipe(const std::string& json) {
    nlohmann::json j = nlohmann::json::parse(json);

    recipe recipe(
        j["title"].get<std::string>(),
        j["ingredients"].get<std::string>(),
        j["instructions"].get<std::string>(),
        j["servingSize"].get<std::string>(),
        j["cookTime"].get<std::string>(),
        j["category"].get<std::string>(),
        j["type"].get<std::string>(),
        j["id"].get<std::string>()
    );

    return recipe;
}

bool RecipeManagerSQLite::addRecipe(const recipe& recipeParam) {
    std::string id = recipeParam.getId().empty() ? generateId() : recipeParam.getId();
    
    // Create a copy of the recipe with the correct id for JSON serialization
    recipe recipeWithId(recipeParam.getTitle(), recipeParam.getIngredients(), recipeParam.getInstructions(),
                       recipeParam.getServingSize(), recipeParam.getCookTime(), recipeParam.getCategory(), 
                       recipeParam.getType(), id);
    
    std::string jsonData = recipeToJson(recipeWithId);

    const char* insertSQL = "INSERT INTO recipes (id, data) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, jsonData.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::updateRecipe(const std::string& id, const recipe& recipe) {
    std::string jsonData = recipeToJson(recipe);

    const char* updateSQL = "UPDATE recipes SET data = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), updateSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    sqlite3_bind_blob(stmt, 1, jsonData.c_str(), jsonData.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::deleteRecipe(const std::string& id) {
    const char* deleteSQL = "DELETE FROM recipes WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), deleteSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::unique_ptr<recipe> RecipeManagerSQLite::getRecipe(const std::string& id) {
    const char* selectSQL = "SELECT data FROM recipes WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const void* blobData = sqlite3_column_blob(stmt, 0);
        int blobSize = sqlite3_column_bytes(stmt, 0);
        std::string jsonData(static_cast<const char*>(blobData), blobSize);
        sqlite3_finalize(stmt);
        return std::unique_ptr<recipe>(new recipe(jsonToRecipe(jsonData)));
    }

    sqlite3_finalize(stmt);
    return nullptr;
}

std::vector<recipe> RecipeManagerSQLite::getAllRecipes() {
    std::vector<recipe> recipes;
    const char* selectSQL = "SELECT data FROM recipes ORDER BY created_at DESC;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return recipes;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        recipes.push_back(jsonToRecipe(jsonData));
    }

    sqlite3_finalize(stmt);
    return recipes;
}

std::vector<recipe> RecipeManagerSQLite::searchByTitle(const std::string& title) {
    std::vector<recipe> recipes;
    const char* searchSQL = "SELECT data FROM recipes WHERE data LIKE ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), searchSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return recipes;
    }

    std::string searchPattern = "%" + title + "%";
    sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string jsonStr = jsonData;

        // Check if title matches (simple string search in JSON)
        if (jsonStr.find(title) != std::string::npos) {
            recipes.push_back(jsonToRecipe(jsonStr));
        }
    }

    sqlite3_finalize(stmt);
    return recipes;
}

std::vector<recipe> RecipeManagerSQLite::searchByCategory(const std::string& category) {
    // Similar implementation to searchByTitle
    return searchByTitle(category); // Simplified - could be more sophisticated
}

std::vector<recipe> RecipeManagerSQLite::searchByType(const std::string& type) {
    // Similar implementation to searchByTitle
    return searchByTitle(type); // Simplified - could be more sophisticated
}