#include "recipeManagerSQLite.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <cctype>
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

    const char* createRecipesTableSQL =
        "CREATE TABLE IF NOT EXISTS recipes ("
        "id TEXT PRIMARY KEY,"
        "data BLOB NOT NULL,"
        "user_id TEXT,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL"
        ");";

    const char* createUsersTableSQL =
        "CREATE TABLE IF NOT EXISTS users ("
        "id TEXT PRIMARY KEY,"
        "email TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "is_active INTEGER DEFAULT 1,"
        "name TEXT DEFAULT '',"
        "bio TEXT DEFAULT '',"
        "avatar_url TEXT DEFAULT '',"
        "preferences TEXT DEFAULT '{}',"
        "privacy_settings TEXT DEFAULT '{}'"
        ");";

    const char* createCollectionsTableSQL =
        "CREATE TABLE IF NOT EXISTS collections ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "description TEXT DEFAULT '',"
        "user_id TEXT NOT NULL,"
        "privacy_settings TEXT DEFAULT '{}',"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    const char* createCollectionRecipesTableSQL =
        "CREATE TABLE IF NOT EXISTS collection_recipes ("
        "collection_id TEXT NOT NULL,"
        "recipe_id TEXT NOT NULL,"
        "added_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "PRIMARY KEY (collection_id, recipe_id),"
        "FOREIGN KEY (collection_id) REFERENCES collections(id) ON DELETE CASCADE,"
        "FOREIGN KEY (recipe_id) REFERENCES recipes(id) ON DELETE CASCADE"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createRecipesTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create recipes table: " + error);
    }

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createUsersTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create users table: " + error);
    }

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createCollectionsTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create collections table: " + error);
    }

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createCollectionRecipesTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create collection_recipes table: " + error);
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

std::vector<recipe> RecipeManagerSQLite::advancedSearch(const SearchCriteria& criteria) {
    std::vector<recipe> recipes;
    std::vector<recipe> allRecipes = getAllRecipes();
    
    for (const auto& r : allRecipes) {
        bool matches = true;
        
        // Full-text search across all fields
        if (!criteria.query.empty()) {
            std::string searchLower = criteria.query;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            
            std::string titleLower = r.getTitle();
            std::string ingredientsLower = r.getIngredients();
            std::string instructionsLower = r.getInstructions();
            std::string categoryLower = r.getCategory();
            std::string typeLower = r.getType();
            
            std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            std::transform(ingredientsLower.begin(), ingredientsLower.end(), ingredientsLower.begin(), ::tolower);
            std::transform(instructionsLower.begin(), instructionsLower.end(), instructionsLower.begin(), ::tolower);
            std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);
            std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
            
            bool found = titleLower.find(searchLower) != std::string::npos ||
                        ingredientsLower.find(searchLower) != std::string::npos ||
                        instructionsLower.find(searchLower) != std::string::npos ||
                        categoryLower.find(searchLower) != std::string::npos ||
                        typeLower.find(searchLower) != std::string::npos;
                        
            if (!found) matches = false;
        }
        
        // Filter by category
        if (!criteria.category.empty() && matches) {
            std::string categoryLower = r.getCategory();
            std::string criteriaLower = criteria.category;
            std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);
            std::transform(criteriaLower.begin(), criteriaLower.end(), criteriaLower.begin(), ::tolower);
            
            if (categoryLower.find(criteriaLower) == std::string::npos) {
                matches = false;
            }
        }
        
        // Filter by type (meal type)
        if (!criteria.type.empty() && matches) {
            std::string typeLower = r.getType();
            std::string criteriaLower = criteria.type;
            std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
            std::transform(criteriaLower.begin(), criteriaLower.end(), criteriaLower.begin(), ::tolower);
            
            if (typeLower.find(criteriaLower) == std::string::npos) {
                matches = false;
            }
        }
        
        // Filter by ingredient
        if (!criteria.ingredient.empty() && matches) {
            std::string ingredientsLower = r.getIngredients();
            std::string criteriaLower = criteria.ingredient;
            std::transform(ingredientsLower.begin(), ingredientsLower.end(), ingredientsLower.begin(), ::tolower);
            std::transform(criteriaLower.begin(), criteriaLower.end(), criteriaLower.begin(), ::tolower);
            
            if (ingredientsLower.find(criteriaLower) == std::string::npos) {
                matches = false;
            }
        }
        
        // Filter by cook time (max)
        if (!criteria.cookTimeMax.empty() && matches) {
            try {
                int maxTime = std::stoi(criteria.cookTimeMax);
                std::string cookTime = r.getCookTime();
                // Extract numeric value from cook time string (e.g., "30 minutes" -> 30)
                int recipeTime = 0;
                size_t pos = cookTime.find_first_of("0123456789");
                if (pos != std::string::npos) {
                    recipeTime = std::stoi(cookTime.substr(pos));
                }
                if (recipeTime > maxTime) {
                    matches = false;
                }
            } catch (...) {
                // Invalid cook time, skip this filter
            }
        }
        
        // Filter by serving size (min/max)
        if ((!criteria.servingSizeMin.empty() || !criteria.servingSizeMax.empty()) && matches) {
            try {
                std::string servingSize = r.getServingSize();
                int size = 0;
                size_t pos = servingSize.find_first_of("0123456789");
                if (pos != std::string::npos) {
                    size = std::stoi(servingSize.substr(pos));
                }
                
                if (!criteria.servingSizeMin.empty()) {
                    int minSize = std::stoi(criteria.servingSizeMin);
                    if (size < minSize) matches = false;
                }
                
                if (!criteria.servingSizeMax.empty()) {
                    int maxSize = std::stoi(criteria.servingSizeMax);
                    if (size > maxSize) matches = false;
                }
            } catch (...) {
                // Invalid serving size, skip this filter
            }
        }
        
        if (matches) {
            recipes.push_back(r);
        }
    }
    
    // Sort results
    if (!criteria.sortBy.empty()) {
        std::string sortBy = criteria.sortBy;
        std::string sortOrder = criteria.sortOrder.empty() ? "asc" : criteria.sortOrder;
        
        std::sort(recipes.begin(), recipes.end(), [&](const recipe& a, const recipe& b) {
            bool ascending = (sortOrder == "asc");
            
            if (sortBy == "title") {
                return ascending ? a.getTitle() < b.getTitle() : a.getTitle() > b.getTitle();
            } else if (sortBy == "cookTime") {
                // Extract numeric cook time for comparison
                auto extractTime = [](const std::string& cookTime) -> int {
                    size_t pos = cookTime.find_first_of("0123456789");
                    if (pos != std::string::npos) {
                        try {
                            return std::stoi(cookTime.substr(pos));
                        } catch (...) {
                            return 0;
                        }
                    }
                    return 0;
                };
                int timeA = extractTime(a.getCookTime());
                int timeB = extractTime(b.getCookTime());
                return ascending ? timeA < timeB : timeA > timeB;
            } else if (sortBy == "category") {
                return ascending ? a.getCategory() < b.getCategory() : a.getCategory() > b.getCategory();
            }
            
            // Default: sort by title
            return ascending ? a.getTitle() < b.getTitle() : a.getTitle() > b.getTitle();
        });
    }
    
    return recipes;
}