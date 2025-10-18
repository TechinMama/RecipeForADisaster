#include "collectionManager.h"
#include "recipe.h"
#include "recipeManagerSQLite.h"
#include <iostream>
#include <sstream>

CollectionManager::CollectionManager(sqlite3* db, const std::string& recipeDbPath)
    : db_(db), recipeDbPath_(recipeDbPath) {}

bool CollectionManager::createCollection(const Collection& collection) {
    const std::string query = R"(
        INSERT INTO collections (id, name, description, user_id, privacy_settings, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )";

    std::vector<std::string> params = {
        collection.getId(),
        collection.getName(),
        collection.getDescription(),
        collection.getUserId(),
        collection.getPrivacySettings()
    };

    return executeQuery(query, params);
}

std::optional<Collection> CollectionManager::findCollectionById(const std::string& id) {
    const std::string query = R"(
        SELECT id, name, description, user_id, privacy_settings, created_at, updated_at
        FROM collections
        WHERE id = ?
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }

    rc = sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    std::optional<Collection> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = collectionFromRow(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<Collection> CollectionManager::findCollectionsByUserId(const std::string& userId) {
    const std::string query = R"(
        SELECT id, name, description, user_id, privacy_settings, created_at, updated_at
        FROM collections
        WHERE user_id = ?
        ORDER BY created_at DESC
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    rc = sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return {};
    }

    std::vector<Collection> collections;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto collection = collectionFromRow(stmt);
        if (collection) {
            collections.push_back(*collection);
        }
    }

    sqlite3_finalize(stmt);
    return collections;
}

std::vector<Collection> CollectionManager::getAllCollections() {
    const std::string query = R"(
        SELECT id, name, description, user_id, privacy_settings, created_at, updated_at
        FROM collections
        ORDER BY created_at DESC
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    std::vector<Collection> collections;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto collection = collectionFromRow(stmt);
        if (collection) {
            collections.push_back(*collection);
        }
    }

    sqlite3_finalize(stmt);
    return collections;
}

bool CollectionManager::updateCollection(const Collection& collection) {
    const std::string query = R"(
        UPDATE collections
        SET name = ?, description = ?, privacy_settings = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    std::vector<std::string> params = {
        collection.getName(),
        collection.getDescription(),
        collection.getPrivacySettings(),
        collection.getId()
    };

    return executeQuery(query, params);
}

bool CollectionManager::deleteCollection(const std::string& id) {
    const std::string query = "DELETE FROM collections WHERE id = ?";
    std::vector<std::string> params = {id};
    return executeQuery(query, params);
}

bool CollectionManager::addRecipeToCollection(const std::string& collectionId, const std::string& recipeId) {
    const std::string query = R"(
        INSERT OR IGNORE INTO collection_recipes (collection_id, recipe_id, added_at)
        VALUES (?, ?, CURRENT_TIMESTAMP)
    )";

    std::vector<std::string> params = {collectionId, recipeId};
    return executeQuery(query, params);
}

bool CollectionManager::removeRecipeFromCollection(const std::string& collectionId, const std::string& recipeId) {
    const std::string query = R"(
        DELETE FROM collection_recipes
        WHERE collection_id = ? AND recipe_id = ?
    )";

    std::vector<std::string> params = {collectionId, recipeId};
    return executeQuery(query, params);
}

std::vector<std::string> CollectionManager::getRecipeIdsInCollection(const std::string& collectionId) {
    const std::string query = R"(
        SELECT recipe_id FROM collection_recipes
        WHERE collection_id = ?
        ORDER BY added_at ASC
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    rc = sqlite3_bind_text(stmt, 1, collectionId.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return {};
    }

    std::vector<std::string> recipeIds;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* recipeId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (recipeId) {
            recipeIds.push_back(std::string(recipeId));
        }
    }

    sqlite3_finalize(stmt);
    return recipeIds;
}

std::vector<std::string> CollectionManager::getCollectionIdsForRecipe(const std::string& recipeId) {
    const std::string query = R"(
        SELECT collection_id FROM collection_recipes
        WHERE recipe_id = ?
        ORDER BY added_at ASC
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    rc = sqlite3_bind_text(stmt, 1, recipeId.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_finalize(stmt);
        return {};
    }

    std::vector<std::string> collectionIds;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* collectionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (collectionId) {
            collectionIds.push_back(std::string(collectionId));
        }
    }

    sqlite3_finalize(stmt);
    return collectionIds;
}

std::optional<Collection> CollectionManager::collectionFromRow(sqlite3_stmt* stmt) const {
    try {
        int columnCount = sqlite3_column_count(stmt);
        if (columnCount < 7) {
            std::cerr << "Not enough columns in result set" << std::endl;
            return std::nullopt;
        }

        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) : "";
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
        std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) : "";
        std::string userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        std::string privacySettings = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "{}";
        std::string createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        std::string updatedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";

        return Collection(name, description, userId, privacySettings, id, createdAt, updatedAt);
    } catch (const std::exception& e) {
        std::cerr << "Error creating collection from row: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool CollectionManager::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to bind parameter " << (i + 1) << ": " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    return true;
}

std::optional<std::string> CollectionManager::executeScalarQuery(const std::string& query, const std::vector<std::string>& params) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return std::nullopt;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); ++i) {
        rc = sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to bind parameter " << (i + 1) << ": " << sqlite3_errmsg(db_) << std::endl;
            sqlite3_finalize(stmt);
            return std::nullopt;
        }
    }

    std::optional<std::string> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (text) {
            result = std::string(text);
        }
    }

    sqlite3_finalize(stmt);
    return result;
}

// Alias methods for compatibility
std::optional<Collection> CollectionManager::getCollectionById(const std::string& id) {
    return findCollectionById(id);
}

std::vector<Collection> CollectionManager::getUserCollections(const std::string& userId) {
    return findCollectionsByUserId(userId);
}

int CollectionManager::getCollectionRecipeCount(const std::string& collectionId) {
    auto recipeIds = getRecipeIdsInCollection(collectionId);
    return static_cast<int>(recipeIds.size());
}

std::vector<recipe> CollectionManager::getCollectionRecipes(const std::string& collectionId) {
    auto recipeIds = getRecipeIdsInCollection(collectionId);
    std::vector<recipe> recipes;

    // Use RecipeManagerSQLite to get full recipe objects
    RecipeManagerSQLite recipeManager(recipeDbPath_);

    for (const auto& recipeId : recipeIds) {
        auto recipePtr = recipeManager.getRecipe(recipeId);
        if (recipePtr) {
            recipes.push_back(*recipePtr);
        }
    }

    return recipes;
}