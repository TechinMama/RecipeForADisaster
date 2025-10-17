#ifndef COLLECTION_MANAGER_H
#define COLLECTION_MANAGER_H

#include "collection.h"
#include "recipe.h"
#include <sqlite3.h>
#include <vector>
#include <optional>
#include <string>

class CollectionManager {
public:
    explicit CollectionManager(sqlite3* db, const std::string& recipeDbPath = "recipes.db");
    ~CollectionManager() = default;

    // Collection CRUD operations
    bool createCollection(const Collection& collection);
    std::optional<Collection> findCollectionById(const std::string& id);
    std::optional<Collection> getCollectionById(const std::string& id); // Alias for findCollectionById
    std::vector<Collection> findCollectionsByUserId(const std::string& userId);
    std::vector<Collection> getUserCollections(const std::string& userId); // Alias for findCollectionsByUserId
    std::vector<Collection> getAllCollections();
    bool updateCollection(const Collection& collection);
    bool deleteCollection(const std::string& id);

    // Collection-Recipe relationship operations
    bool addRecipeToCollection(const std::string& collectionId, const std::string& recipeId);
    bool removeRecipeFromCollection(const std::string& collectionId, const std::string& recipeId);
    std::vector<std::string> getRecipeIdsInCollection(const std::string& collectionId);
    std::vector<std::string> getCollectionIdsForRecipe(const std::string& recipeId);
    int getCollectionRecipeCount(const std::string& collectionId);
    std::vector<recipe> getCollectionRecipes(const std::string& collectionId);

private:
    sqlite3* db_;
    std::string recipeDbPath_;

    // Helper methods
    std::optional<Collection> collectionFromRow(sqlite3_stmt* stmt) const;
    bool executeQuery(const std::string& query, const std::vector<std::string>& params = {});
    std::optional<std::string> executeScalarQuery(const std::string& query, const std::vector<std::string>& params = {});
};

#endif // COLLECTION_MANAGER_H