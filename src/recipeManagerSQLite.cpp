#include <sw/redis++/redis++.h>
#include <sstream>
#include "recipeManagerSQLite.h"
#include <sqlite3.h>

#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <cctype>

// Redis connection (singleton for simplicity)
static sw::redis::Redis& getRedis() {
    static sw::redis::Redis redis("tcp://127.0.0.1:6379");
    return redis;
}

RecipeManagerSQLite::RecipeManagerSQLite(const std::string& dbPath)
    : dbPath_(dbPath), db_(nullptr) {
    initializeDatabase();
}

RecipeManagerSQLite::~RecipeManagerSQLite() {
    if (db_) {
        sqlite3_close(static_cast<sqlite3*>(db_));
        db_ = nullptr;
    }
}

void RecipeManagerSQLite::initializeDatabase() {
    sqlite3* db;
    int rc = sqlite3_open(dbPath_.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        if (db) {
            sqlite3_close(db);
        }
        db_ = nullptr;
        return;
    }
    db_ = db;

    // Create recipes table
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS recipes ("
        "id TEXT PRIMARY KEY,"
        "data TEXT NOT NULL,"
        "user_id TEXT,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create recipes table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    // Create ratings table
    const char* createRatingsSQL = 
        "CREATE TABLE IF NOT EXISTS ratings ("
        "id TEXT PRIMARY KEY,"
        "recipe_id TEXT NOT NULL,"
        "user_id TEXT NOT NULL,"
        "rating INTEGER NOT NULL CHECK(rating >= 1 AND rating <= 5),"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(recipe_id) REFERENCES recipes(id),"
        "UNIQUE(recipe_id, user_id)"
        ");";

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createRatingsSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create ratings table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    // Create reviews table
    const char* createReviewsSQL = 
        "CREATE TABLE IF NOT EXISTS reviews ("
        "id TEXT PRIMARY KEY,"
        "recipe_id TEXT NOT NULL,"
        "user_id TEXT NOT NULL,"
        "rating INTEGER NOT NULL CHECK(rating >= 1 AND rating <= 5),"
        "review_text TEXT NOT NULL,"
        "status TEXT DEFAULT 'pending',"
        "moderation_reason TEXT,"
        "helpful_votes INTEGER DEFAULT 0,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(recipe_id) REFERENCES recipes(id),"
        "UNIQUE(recipe_id, user_id)"
        ");";

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createReviewsSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create reviews table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    // Create review_votes table
    const char* createReviewVotesSQL = 
        "CREATE TABLE IF NOT EXISTS review_votes ("
        "review_id TEXT NOT NULL,"
        "user_id TEXT NOT NULL,"
        "vote_type TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "PRIMARY KEY(review_id, user_id),"
        "FOREIGN KEY(review_id) REFERENCES reviews(id)"
        ");";

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createReviewVotesSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to create review_votes table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool RecipeManagerSQLite::addRecipe(const recipe& recipe) {
    return addRecipe(recipe, "");
}

bool RecipeManagerSQLite::addRecipe(const recipe& recipe, const std::string& userId) {
    const char* sql = userId.empty() 
        ? "INSERT INTO recipes (id, data) VALUES (?, ?);"
        : "INSERT INTO recipes (id, data, user_id) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    std::string id = recipe.getId().empty() ? generateId() : recipe.getId();
    // Create a copy of the recipe with the generated ID
    ::recipe recipeWithId(recipe.getTitle(), recipe.getIngredients(), recipe.getInstructions(),
                       recipe.getServingSize(), recipe.getCookTime(), recipe.getCategory(), 
                       recipe.getType(), id);
    std::string jsonData = recipeToJson(recipeWithId);

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, jsonData.c_str(), -1, SQLITE_TRANSIENT);
    if (!userId.empty()) {
        sqlite3_bind_text(stmt, 3, userId.c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::updateRecipe(const std::string& id, const recipe& recipe) {
    const char* sql = "UPDATE recipes SET data = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    std::string jsonData = recipeToJson(recipe);

    sqlite3_bind_text(stmt, 1, jsonData.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::updateRecipeByTitle(const std::string& title, const recipe& recipe) {
    const char* sql = "UPDATE recipes SET data = ?, updated_at = CURRENT_TIMESTAMP WHERE json_extract(data, '$.title') = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    std::string jsonData = recipeToJson(recipe);

    sqlite3_bind_text(stmt, 1, jsonData.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::string RecipeManagerSQLite::generateId() {
    // Simple ID generation - in production, use UUID
    static int counter = 0;
    return "recipe_" + std::to_string(++counter) + "_" + std::to_string(std::time(nullptr));
}

std::string RecipeManagerSQLite::recipeToJson(const recipe& recipe) {
    return recipe.toJson();
}

bool RecipeManagerSQLite::isConnected() const {
    return db_ != nullptr;
}

std::vector<recipe> RecipeManagerSQLite::advancedSearch(const SearchCriteria& criteria) {
    std::vector<recipe> recipes;
    std::string sql = "SELECT data FROM recipes WHERE 1=1";
    std::vector<std::string> params;

    // Determine if this is an expensive query (full-text search or multiple filters)
    bool isExpensive = !criteria.query.empty() ||
                      (!criteria.category.empty() && !criteria.type.empty()) ||
                      (!criteria.ingredient.empty()) ||
                      (!criteria.cookTimeMax.empty()) ||
                      (!criteria.servingSizeMin.empty() || !criteria.servingSizeMax.empty());

    // Check cache for expensive queries
    std::string cacheKey;
    if (isExpensive) {
        std::ostringstream oss;
        oss << "advsearch:" << criteria.query << ":" << criteria.category << ":" << criteria.type
            << ":" << criteria.ingredient << ":" << criteria.cookTimeMax << ":" << criteria.servingSizeMin
            << ":" << criteria.servingSizeMax << ":" << criteria.sortBy << ":" << criteria.sortOrder;
        cacheKey = oss.str();
        auto& redis = getRedis();
        auto cached = redis.get(cacheKey);
        if (cached) {
            try {
                nlohmann::json arr = nlohmann::json::parse(*cached);
                for (const auto& item : arr) {
                    recipes.push_back(jsonToRecipe(item.dump()));
                }
                return recipes;
            } catch (...) {
                // Invalid cache, continue with query
            }
        }
    }

    // Build WHERE conditions
    if (!criteria.query.empty()) {
        // Full-text search across multiple fields using LIKE
        std::string queryLower = criteria.query;
        std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);
        sql += " AND (LOWER(json_extract(data, '$.title')) LIKE ? OR "
               "LOWER(json_extract(data, '$.ingredients')) LIKE ? OR "
               "LOWER(json_extract(data, '$.instructions')) LIKE ? OR "
               "LOWER(json_extract(data, '$.category')) LIKE ? OR "
               "LOWER(json_extract(data, '$.type')) LIKE ?)";
        std::string likeQuery = "%" + queryLower + "%";
        params.insert(params.end(), {likeQuery, likeQuery, likeQuery, likeQuery, likeQuery});
    }

    if (!criteria.category.empty()) {
        sql += " AND LOWER(json_extract(data, '$.category')) LIKE ?";
        params.push_back("%" + criteria.category + "%");
    }

    if (!criteria.type.empty()) {
        sql += " AND LOWER(json_extract(data, '$.type')) LIKE ?";
        params.push_back("%" + criteria.type + "%");
    }

    if (!criteria.ingredient.empty()) {
        sql += " AND LOWER(json_extract(data, '$.ingredients')) LIKE ?";
        params.push_back("%" + criteria.ingredient + "%");
    }

    if (!criteria.cookTimeMax.empty()) {
        // Extract numeric cook time and compare
        sql += " AND CAST(SUBSTR(json_extract(data, '$.cookTime'), 1, INSTR(json_extract(data, '$.cookTime'), ' ') - 1) AS INTEGER) <= ?";
        params.push_back(criteria.cookTimeMax);
    }

    if (!criteria.servingSizeMin.empty()) {
        sql += " AND CAST(SUBSTR(json_extract(data, '$.servingSize'), 1, INSTR(json_extract(data, '$.servingSize'), ' ') - 1) AS INTEGER) >= ?";
        params.push_back(criteria.servingSizeMin);
    }

    if (!criteria.servingSizeMax.empty()) {
        sql += " AND CAST(SUBSTR(json_extract(data, '$.servingSize'), 1, INSTR(json_extract(data, '$.servingSize'), ' ') - 1) AS INTEGER) <= ?";
        params.push_back(criteria.servingSizeMax);
    }

    // Add sorting
    if (!criteria.sortBy.empty()) {
        std::string sortOrder = criteria.sortOrder.empty() ? "ASC" : (criteria.sortOrder == "desc" ? "DESC" : "ASC");

        if (criteria.sortBy == "title") {
            sql += " ORDER BY LOWER(json_extract(data, '$.title')) " + sortOrder;
        } else if (criteria.sortBy == "cookTime") {
            sql += " ORDER BY CAST(SUBSTR(json_extract(data, '$.cookTime'), 1, INSTR(json_extract(data, '$.cookTime'), ' ') - 1) AS INTEGER) " + sortOrder;
        } else if (criteria.sortBy == "category") {
            sql += " ORDER BY LOWER(json_extract(data, '$.category')) " + sortOrder;
        } else if (criteria.sortBy == "createdAt") {
            sql += " ORDER BY created_at " + sortOrder;
        }
    } else {
        // Default sort by title
        sql += " ORDER BY LOWER(json_extract(data, '$.title')) ASC";
    }

    // Execute query
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare advanced search statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return recipes;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    nlohmann::json resultArr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* data = sqlite3_column_text(stmt, 0);
        if (data) {
            std::string jsonStr = reinterpret_cast<const char*>(data);
            recipes.push_back(jsonToRecipe(jsonStr));
            if (isExpensive) {
                resultArr.push_back(nlohmann::json::parse(jsonStr));
            }
        }
    }
    sqlite3_finalize(stmt);

    // Cache results for expensive queries
    if (isExpensive && !recipes.empty()) {
        auto& redis = getRedis();
        redis.set(cacheKey, resultArr.dump());
        redis.expire(cacheKey, 300); // 5 min TTL
    }

    return recipes;
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


// User-specific operations
bool RecipeManagerSQLite::isRecipeOwnedByUser(const std::string& recipeId, const std::string& userId) {
    const char* selectSQL = "SELECT COUNT(*) FROM recipes WHERE id = ? AND user_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);

    bool owned = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        owned = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return owned;
}

bool RecipeManagerSQLite::isRecipeOwnedByUserByTitle(const std::string& recipeTitle, const std::string& userId) {
    const char* selectSQL = "SELECT COUNT(*) FROM recipes r WHERE json_extract(r.data, '$.title') = ? AND r.user_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, recipeTitle.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);

    bool owned = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        owned = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return owned;
}

std::vector<recipe> RecipeManagerSQLite::getRecipesByUser(const std::string& userId) {
    const char* selectSQL = "SELECT data FROM recipes WHERE user_id = ? ORDER BY created_at DESC;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<recipe> recipes;

    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), selectSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(static_cast<sqlite3*>(db_)) << std::endl;
        return recipes;
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const void* blobData = sqlite3_column_blob(stmt, 0);
        int blobSize = sqlite3_column_bytes(stmt, 0);
        std::string jsonData(static_cast<const char*>(blobData), blobSize);
        recipes.push_back(jsonToRecipe(jsonData));
    }

    sqlite3_finalize(stmt);
    return recipes;
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
        const char* jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string jsonStr = jsonData ? jsonData : "";
        sqlite3_finalize(stmt);
                return std::unique_ptr<recipe>(new recipe(jsonToRecipe(jsonStr)));
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

// Rating operations
bool RecipeManagerSQLite::addOrUpdateRating(const std::string& recipeId, const std::string& userId, int rating) {
    if (rating < 1 || rating > 5) {
        return false;
    }

    const char* sql = 
        "INSERT OR REPLACE INTO ratings (id, recipe_id, user_id, rating, updated_at) "
        "VALUES (?, ?, ?, ?, CURRENT_TIMESTAMP)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    std::string ratingId = "rating_" + recipeId + "_" + userId;

    sqlite3_bind_text(stmt, 1, ratingId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, recipeId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, rating);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::deleteRating(const std::string& recipeId, const std::string& userId) {
    const char* sql = "DELETE FROM ratings WHERE recipe_id = ? AND user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::unique_ptr<RecipeManagerSQLite::Rating> RecipeManagerSQLite::getRating(const std::string& recipeId, const std::string& userId) {
    const char* sql = 
        "SELECT id, recipe_id, user_id, rating, created_at, updated_at "
        "FROM ratings WHERE recipe_id = ? AND user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto rating = std::make_unique<Rating>();
        rating->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rating->recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rating->userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        rating->rating = sqlite3_column_int(stmt, 3);
        rating->createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rating->updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        sqlite3_finalize(stmt);
        return rating;
    }

    sqlite3_finalize(stmt);
    return nullptr;
}

double RecipeManagerSQLite::getAverageRating(const std::string& recipeId) {
    const char* sql = "SELECT AVG(rating) FROM ratings WHERE recipe_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0.0;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);

    double avg = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        avg = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return avg;
}

int RecipeManagerSQLite::getRatingCount(const std::string& recipeId) {
    const char* sql = "SELECT COUNT(*) FROM ratings WHERE recipe_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

std::vector<RecipeManagerSQLite::Rating> RecipeManagerSQLite::getRatingsByRecipe(const std::string& recipeId) {
    std::vector<Rating> ratings;

    const char* sql = 
        "SELECT id, recipe_id, user_id, rating, created_at, updated_at "
        "FROM ratings WHERE recipe_id = ? ORDER BY created_at DESC";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return ratings;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Rating rating;
        rating.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rating.recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rating.userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        rating.rating = sqlite3_column_int(stmt, 3);
        rating.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rating.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        ratings.push_back(rating);
    }

    sqlite3_finalize(stmt);
    return ratings;
}

std::vector<RecipeManagerSQLite::Rating> RecipeManagerSQLite::getRatingsByUser(const std::string& userId) {
    std::vector<Rating> ratings;

    const char* sql = 
        "SELECT id, recipe_id, user_id, rating, created_at, updated_at "
        "FROM ratings WHERE user_id = ? ORDER BY created_at DESC";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return ratings;
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Rating rating;
        rating.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rating.recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rating.userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        rating.rating = sqlite3_column_int(stmt, 3);
        rating.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rating.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        ratings.push_back(rating);
    }

    sqlite3_finalize(stmt);
    return ratings;
}

// Review operations
bool RecipeManagerSQLite::addReview(const Review& review) {
    if (review.rating < 1 || review.rating > 5 || review.reviewText.length() > 500) {
        return false;
    }

    const char* sql = 
        "INSERT INTO reviews (id, recipe_id, user_id, rating, review_text, status) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    std::string reviewId = "review_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, review.recipeId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, review.userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, review.rating);
    sqlite3_bind_text(stmt, 5, review.reviewText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, review.status.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::updateReview(const std::string& reviewId, const Review& review) {
    if (review.rating < 1 || review.rating > 5 || review.reviewText.length() > 500) {
        return false;
    }

    const char* sql = 
        "UPDATE reviews SET rating = ?, review_text = ?, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, review.rating);
    sqlite3_bind_text(stmt, 2, review.reviewText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, reviewId.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::deleteReview(const std::string& reviewId) {
    const char* sql = "DELETE FROM reviews WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::unique_ptr<RecipeManagerSQLite::Review> RecipeManagerSQLite::getReview(const std::string& reviewId) {
    const char* sql = 
        "SELECT id, recipe_id, user_id, rating, review_text, status, moderation_reason, helpful_votes, created_at, updated_at "
        "FROM reviews WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto review = std::make_unique<Review>();
        review->id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        review->recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        review->userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        review->rating = sqlite3_column_int(stmt, 3);
        review->reviewText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        review->status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        review->moderationReason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        review->helpfulVotes = sqlite3_column_int(stmt, 7);
        review->createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        review->updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        sqlite3_finalize(stmt);
        return review;
    }

    sqlite3_finalize(stmt);
    return nullptr;
}

std::vector<RecipeManagerSQLite::Review> RecipeManagerSQLite::getReviewsByRecipe(const std::string& recipeId, const std::string& status) {
    std::vector<Review> reviews;

    std::string sql = 
        "SELECT id, recipe_id, user_id, rating, review_text, status, moderation_reason, helpful_votes, created_at, updated_at "
        "FROM reviews WHERE recipe_id = ?";
    
    if (!status.empty()) {
        sql += " AND status = ?";
    }
    sql += " ORDER BY created_at DESC";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return reviews;
    }

    sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);
    if (!status.empty()) {
        sqlite3_bind_text(stmt, 2, status.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Review review;
        review.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        review.recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        review.userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        review.rating = sqlite3_column_int(stmt, 3);
        review.reviewText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        review.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        review.moderationReason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        review.helpfulVotes = sqlite3_column_int(stmt, 7);
        review.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        review.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        reviews.push_back(review);
    }

    sqlite3_finalize(stmt);
    return reviews;
}

std::vector<RecipeManagerSQLite::Review> RecipeManagerSQLite::getReviewsByUser(const std::string& userId) {
    std::vector<Review> reviews;

    const char* sql = 
        "SELECT id, recipe_id, user_id, rating, review_text, status, moderation_reason, helpful_votes, created_at, updated_at "
        "FROM reviews WHERE user_id = ? ORDER BY created_at DESC";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return reviews;
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Review review;
        review.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        review.recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        review.userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        review.rating = sqlite3_column_int(stmt, 3);
        review.reviewText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        review.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        review.moderationReason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        review.helpfulVotes = sqlite3_column_int(stmt, 7);
        review.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        review.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        reviews.push_back(review);
    }

    sqlite3_finalize(stmt);
    return reviews;
}

std::vector<RecipeManagerSQLite::Review> RecipeManagerSQLite::getPendingReviews() {
    std::vector<Review> reviews;

    const char* sql = 
        "SELECT id, recipe_id, user_id, rating, review_text, status, moderation_reason, helpful_votes, created_at, updated_at "
        "FROM reviews WHERE status = 'pending' ORDER BY created_at ASC";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return reviews;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Review review;
        review.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        review.recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        review.userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        review.rating = sqlite3_column_int(stmt, 3);
        review.reviewText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        review.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        review.moderationReason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        review.helpfulVotes = sqlite3_column_int(stmt, 7);
        review.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        review.updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        reviews.push_back(review);
    }

    sqlite3_finalize(stmt);
    return reviews;
}

bool RecipeManagerSQLite::moderateReview(const std::string& reviewId, const std::string& status, const std::string& reason) {
    if (status != "approved" && status != "rejected") {
        return false;
    }

    const char* sql = 
        "UPDATE reviews SET status = ?, moderation_reason = ?, updated_at = CURRENT_TIMESTAMP "
        "WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, reviewId.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

// Review voting operations
bool RecipeManagerSQLite::addOrUpdateReviewVote(const std::string& reviewId, const std::string& userId, const std::string& voteType) {
    if (voteType != "helpful" && voteType != "not_helpful") {
        return false;
    }

    const char* sql = 
        "INSERT OR REPLACE INTO review_votes (review_id, user_id, vote_type, created_at) "
        "VALUES (?, ?, ?, CURRENT_TIMESTAMP)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, voteType.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Update helpful_votes count in reviews table
    if (rc == SQLITE_DONE) {
        updateHelpfulVotesCount(reviewId);
    }

    return rc == SQLITE_DONE;
}

bool RecipeManagerSQLite::deleteReviewVote(const std::string& reviewId, const std::string& userId) {
    const char* sql = "DELETE FROM review_votes WHERE review_id = ? AND user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Update helpful_votes count in reviews table
    if (rc == SQLITE_DONE) {
        updateHelpfulVotesCount(reviewId);
    }

    return rc == SQLITE_DONE;
}

std::unique_ptr<RecipeManagerSQLite::ReviewVote> RecipeManagerSQLite::getReviewVote(const std::string& reviewId, const std::string& userId) {
    const char* sql = 
        "SELECT review_id, user_id, vote_type, created_at "
        "FROM review_votes WHERE review_id = ? AND user_id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto vote = std::make_unique<ReviewVote>();
        vote->reviewId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        vote->userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        vote->voteType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        vote->createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        sqlite3_finalize(stmt);
        return vote;
    }

    sqlite3_finalize(stmt);
    return nullptr;
}

int RecipeManagerSQLite::getHelpfulVoteCount(const std::string& reviewId) {
    const char* sql = "SELECT COUNT(*) FROM review_votes WHERE review_id = ? AND vote_type = 'helpful'";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, reviewId.c_str(), -1, SQLITE_TRANSIENT);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

void RecipeManagerSQLite::updateHelpfulVotesCount(const std::string& reviewId) {
    int count = getHelpfulVoteCount(reviewId);

    const char* sql = "UPDATE reviews SET helpful_votes = ? WHERE id = ?";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return;
    }

    sqlite3_bind_int(stmt, 1, count);
    sqlite3_bind_text(stmt, 2, reviewId.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// Review sorting and filtering
std::vector<RecipeManagerSQLite::Review> RecipeManagerSQLite::getSortedReviewsByRecipe(const std::string& recipeId, ReviewSortBy sortBy, const std::string& status) {
    std::vector<Review> reviews = getReviewsByRecipe(recipeId, status);

    std::sort(reviews.begin(), reviews.end(), [sortBy](const Review& a, const Review& b) {
        switch (sortBy) {
            case NEWEST:
                return a.createdAt > b.createdAt; // Descending order for newest first
            case OLDEST:
                return a.createdAt < b.createdAt; // Ascending order for oldest first
            case HIGHEST_RATED:
                return a.rating > b.rating; // Descending order for highest rated
            case MOST_HELPFUL:
                return a.helpfulVotes > b.helpfulVotes; // Descending order for most helpful
            default:
                return a.createdAt > b.createdAt; // Default to newest
        }
    });

    return reviews;
}

recipe RecipeManagerSQLite::jsonToRecipe(const std::string& json) {
    return recipe::fromJson(json);
}