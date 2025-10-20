#include <sw/redis++/redis++.h>
#include <sstream>
    // Redis connection (singleton for simplicity)
    static sw::redis::Redis& getRedis() {
        static sw::redis::Redis redis("tcp://127.0.0.1:6379");
        return redis;
    }
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

    const char* createRatingsTableSQL =
        "CREATE TABLE IF NOT EXISTS ratings ("
        "id TEXT PRIMARY KEY,"
        "recipe_id TEXT NOT NULL,"
        "user_id TEXT NOT NULL,"
        "rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(recipe_id, user_id),"
        "FOREIGN KEY (recipe_id) REFERENCES recipes(id) ON DELETE CASCADE,"
        "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    const char* createReviewsTableSQL =
        "CREATE TABLE IF NOT EXISTS reviews ("
        "id TEXT PRIMARY KEY,"
        "recipe_id TEXT NOT NULL,"
        "user_id TEXT NOT NULL,"
        "rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),"
        "review_text TEXT NOT NULL CHECK (LENGTH(review_text) <= 500),"
        "status TEXT DEFAULT 'pending' CHECK (status IN ('pending', 'approved', 'rejected')),"
        "moderation_reason TEXT DEFAULT '',"
        "helpful_votes INTEGER DEFAULT 0,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (recipe_id) REFERENCES recipes(id) ON DELETE CASCADE,"
        "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    const char* createReviewVotesTableSQL =
        "CREATE TABLE IF NOT EXISTS review_votes ("
        "review_id TEXT NOT NULL,"
        "user_id TEXT NOT NULL,"
        "vote_type TEXT NOT NULL CHECK (vote_type IN ('helpful', 'not_helpful')),"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "PRIMARY KEY (review_id, user_id),"
        "FOREIGN KEY (review_id) REFERENCES reviews(id) ON DELETE CASCADE,"
        "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createRecipesTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create recipes table: " + error);
    }

    // Create indexes for performance
    const char* createIndexTitle = "CREATE INDEX IF NOT EXISTS idx_recipes_title ON recipes((json_extract(data, '$.title')));";
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createIndexTitle, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create index on title: " + error);
    }

    const char* createIndexCategory = "CREATE INDEX IF NOT EXISTS idx_recipes_category ON recipes((json_extract(data, '$.category')));";
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createIndexCategory, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create index on category: " + error);
    }

    const char* createIndexType = "CREATE INDEX IF NOT EXISTS idx_recipes_type ON recipes((json_extract(data, '$.type')));";
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createIndexType, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create index on type: " + error);
    }

    const char* createIndexUserId = "CREATE INDEX IF NOT EXISTS idx_recipes_user_id ON recipes(user_id);";
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createIndexUserId, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create index on user_id: " + error);
    }

    const char* createIndexCreatedAt = "CREATE INDEX IF NOT EXISTS idx_recipes_created_at ON recipes(created_at);";
    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createIndexCreatedAt, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create index on created_at: " + error);
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

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createRatingsTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create ratings table: " + error);
    }

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createReviewsTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create reviews table: " + error);
    }

    rc = sqlite3_exec(static_cast<sqlite3*>(db_), createReviewVotesTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create review_votes table: " + error);
    }
}

bool RecipeManagerSQLite::isConnected() const {
    return db_ != nullptr;
}

std::string RecipeManagerSQLite::generateId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::stringstream ss;
    std::vector<recipe> recipes;
    std::string sql = "SELECT data FROM recipes WHERE 1=1";
    std::vector<std::string> conditions;
    std::vector<std::string> params;

    bool isExpensive = false;
    // Full-text search (across title, ingredients, instructions, category, type)
    if (!criteria.query.empty()) {
        conditions.push_back("(LOWER(json_extract(data, '$.title')) LIKE ? OR LOWER(json_extract(data, '$.ingredients')) LIKE ? OR LOWER(json_extract(data, '$.instructions')) LIKE ? OR LOWER(json_extract(data, '$.category')) LIKE ? OR LOWER(json_extract(data, '$.type')) LIKE ?)");
        std::string q = "%" + criteria.query + "%";
        for (int i = 0; i < 5; ++i) params.push_back(q);
        isExpensive = true;
    }
    // Category filter
    if (!criteria.category.empty()) {
        conditions.push_back("LOWER(json_extract(data, '$.category')) LIKE ?");
        params.push_back("%" + criteria.category + "%");
    }
    // Type filter
    if (!criteria.type.empty()) {
        conditions.push_back("LOWER(json_extract(data, '$.type')) LIKE ?");
        params.push_back("%" + criteria.type + "%");
    }
    // Ingredient filter
    if (!criteria.ingredient.empty()) {
        conditions.push_back("LOWER(json_extract(data, '$.ingredients')) LIKE ?");
        params.push_back("%" + criteria.ingredient + "%");
    }
    // Cook time max
    if (!criteria.cookTimeMax.empty()) {
        conditions.push_back("CAST(json_extract(data, '$.cookTime') AS INTEGER) <= ?");
        params.push_back(criteria.cookTimeMax);
    }
    // Serving size min/max
    if (!criteria.servingSizeMin.empty()) {
        conditions.push_back("CAST(json_extract(data, '$.servingSize') AS INTEGER) >= ?");
        params.push_back(criteria.servingSizeMin);
    }
    if (!criteria.servingSizeMax.empty()) {
        conditions.push_back("CAST(json_extract(data, '$.servingSize') AS INTEGER) <= ?");
        params.push_back(criteria.servingSizeMax);
    }

    // Build WHERE clause
    for (const auto& cond : conditions) {
        sql += " AND " + cond;
    }

    // Sorting
    if (!criteria.sortBy.empty()) {
        std::string sortCol;
        if (criteria.sortBy == "title") sortCol = "json_extract(data, '$.title')";
        else if (criteria.sortBy == "cookTime") sortCol = "CAST(json_extract(data, '$.cookTime') AS INTEGER)";
        else if (criteria.sortBy == "category") sortCol = "json_extract(data, '$.category')";
        else sortCol = "json_extract(data, '$.title')";
        std::string sortOrder = criteria.sortOrder.empty() ? "ASC" : criteria.sortOrder;
        sql += " ORDER BY " + sortCol + " " + sortOrder;
    }

    // Only cache expensive queries (full-text search or large result sets)
    std::string cacheKey;
    if (isExpensive) {
        std::ostringstream oss;
        oss << "advsearch:" << criteria.query << ":" << criteria.category << ":" << criteria.type << ":" << criteria.ingredient << ":" << criteria.cookTimeMax << ":" << criteria.servingSizeMin << ":" << criteria.servingSizeMax << ":" << criteria.sortBy << ":" << criteria.sortOrder;
        cacheKey = oss.str();
        auto& redis = getRedis();
        auto cached = redis.get(cacheKey);
        if (cached) {
            // Deserialize cached JSON array
            nlohmann::json arr = nlohmann::json::parse(*cached);
            for (const auto& item : arr) {
                recipe r;
                r.fromJson(item.dump());
                recipes.push_back(r);
            }
            return recipes;
        }
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(static_cast<sqlite3*>(db_), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return recipes;
    }
    int idx = 1;
    for (const auto& p : params) {
        sqlite3_bind_text(stmt, idx++, p.c_str(), -1, SQLITE_TRANSIENT);
    }
    nlohmann::json resultArr = nlohmann::json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* data = sqlite3_column_text(stmt, 0);
        if (data) {
            recipe r;
            r.fromJson(reinterpret_cast<const char*>(data));
            recipes.push_back(r);
            resultArr.push_back(nlohmann::json::parse(reinterpret_cast<const char*>(data)));
        }
    }
    sqlite3_finalize(stmt);

    // Store in Redis if expensive
    if (isExpensive && !recipes.empty()) {
        auto& redis = getRedis();
        redis.set(cacheKey, resultArr.dump());
        redis.expire(cacheKey, 300); // 5 min TTL
    }
    return recipes;
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);

    std::string recipeId;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);

    if (recipeId.empty()) {
        return false; // Recipe not found
    }

    return deleteRecipe(recipeId);
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