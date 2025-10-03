#include "recipeManager.h"
#include <mongocxx/v_noabi/mongocxx/exception/exception.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/json.hpp>
#include <iostream>
#include "vaultService.h"  // Include Vault service header

recipeManager::recipeManager(const std::string& uri)
{
    try {
        if (uri.empty()) {
            throw DatabaseError("MongoDB URI cannot be empty");
        }
        client = mongocxx::client{mongocxx::uri{uri}};
        db = client["RecipeManagerDB"];

        // Test connection
        if (!isConnected()) {
            throw DatabaseError("Failed to connect to MongoDB database");
        }
    } catch (const mongocxx::exception& e) {
        throw DatabaseError("MongoDB connection failed: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw DatabaseError("Database initialization failed: " + std::string(e.what()));
    }
}

recipeManager::recipeManager(VaultService* vaultService, const std::string& vaultPath)
{
    try {
        if (!vaultService) {
            throw DatabaseError("Vault service is required for secure credential retrieval");
        }

        // Retrieve MongoDB URI from Vault
        auto uriResult = vaultService->getSecret(vaultPath, "uri");
        if (!uriResult.success) {
            throw DatabaseError("Failed to retrieve MongoDB URI from Vault: " + uriResult.errorMessage);
        }

        std::string uri = uriResult.value;
        if (uri.empty()) {
            throw DatabaseError("Retrieved MongoDB URI from Vault is empty");
        }

        client = mongocxx::client{mongocxx::uri{uri}};
        db = client["RecipeManagerDB"];

        // Test connection
        if (!isConnected()) {
            throw DatabaseError("Failed to connect to MongoDB database");
        }

        std::cout << "Database connected successfully using Vault credentials" << std::endl;
    } catch (const mongocxx::exception& e) {
        throw DatabaseError("MongoDB connection failed: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw DatabaseError("Database initialization failed: " + std::string(e.what()));
    }
}

recipeManager::OperationResult recipeManager::addRecipe(const recipe& recipe) {
    try {
        auto collection = db["recipes"];
        bsoncxx::builder::stream::document document{};

        document << "title" << recipe.getTitle()
                 << "ingredients" << recipe.getIngredients()
                 << "instructions" << recipe.getInstructions()
                 << "servingSize" << recipe.getServingSize()
                 << "cookTime" << recipe.getCookTime()
                 << "category" << recipe.getCategory()
                 << "type" << recipe.getType();

        auto result = collection.insert_one(document.view());

        if (result) {
            return OperationResult(true, "");
        } else {
            return OperationResult(false, "Failed to insert recipe into database");
        }
    } catch (const mongocxx::exception& e) {
        return OperationResult(false, "Database error while adding recipe: " + std::string(e.what()));
    } catch (const std::exception& e) {
        return OperationResult(false, "Unexpected error while adding recipe: " + std::string(e.what()));
    }
}

std::vector<recipe> recipeManager::viewRecipes() {
    std::vector<recipe> recipes;

    try {
        auto collection = db["recipes"];
        auto cursor = collection.find({});

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string category = safeGetString(doc, "category");
                std::string type = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, category, type);
                recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in database: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error while viewing recipes: " << e.what() << std::endl;
        throw DatabaseError("Failed to retrieve recipes: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error while viewing recipes: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error while retrieving recipes: " + std::string(e.what()));
    }

    return recipes;
}

recipeManager::OperationResult recipeManager::updateRecipe(const std::string& title, const recipe& updatedRecipe) {
    try {
        if (title.empty()) {
            return OperationResult(false, "Recipe title for update cannot be empty");
        }

        auto collection = db["recipes"];
        bsoncxx::builder::stream::document filter{};
        filter << "title" << title;

        bsoncxx::builder::stream::document update{};
        update << "$set" << bsoncxx::builder::stream::open_document
               << "ingredients" << updatedRecipe.getIngredients()
               << "instructions" << updatedRecipe.getInstructions()
               << "servingSize" << updatedRecipe.getServingSize()
               << "cookTime" << updatedRecipe.getCookTime()
               << "category" << updatedRecipe.getCategory()
               << "type" << updatedRecipe.getType()
               << bsoncxx::builder::stream::close_document;

        auto result = collection.update_one(filter.view(), update.view());

        if (result && result->modified_count() > 0) {
            return OperationResult(true, "");
        } else {
            return OperationResult(false, "Recipe not found or no changes made");
        }
    } catch (const mongocxx::exception& e) {
        return OperationResult(false, "Database error while updating recipe: " + std::string(e.what()));
    } catch (const std::exception& e) {
        return OperationResult(false, "Unexpected error while updating recipe: " + std::string(e.what()));
    }
}

recipeManager::OperationResult recipeManager::deleteRecipe(const std::string& title) {
    try {
        if (title.empty()) {
            return OperationResult(false, "Recipe title for deletion cannot be empty");
        }

        auto collection = db["recipes"];
        bsoncxx::builder::stream::document filter{};
        filter << "title" << title;

        auto result = collection.delete_one(filter.view());

        if (result && result->deleted_count() > 0) {
            return OperationResult(true, "");
        } else {
            return OperationResult(false, "Recipe not found");
        }
    } catch (const mongocxx::exception& e) {
        return OperationResult(false, "Database error while deleting recipe: " + std::string(e.what()));
    } catch (const std::exception& e) {
        return OperationResult(false, "Unexpected error while deleting recipe: " + std::string(e.what()));
    }
}

std::vector<recipe> recipeManager::searchRecipes(const std::string& criteria) {
    std::vector<recipe> recipes;

    try {
        if (criteria.empty()) {
            return recipes; // Return empty vector for empty search criteria
        }

        auto collection = db["recipes"];
        bsoncxx::builder::stream::document filter{};
        filter << "title" << bsoncxx::builder::stream::open_document
               << "$regex" << criteria
               << "$options" << "i" // case insensitive
               << bsoncxx::builder::stream::close_document;

        auto cursor = collection.find(filter.view());

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string category = safeGetString(doc, "category");
                std::string type = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, category, type);
                recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in search results: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error while searching recipes: " << e.what() << std::endl;
        throw DatabaseError("Failed to search recipes: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error while searching recipes: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error while searching recipes: " + std::string(e.what()));
    }

    return recipes;
}

// Enhanced search methods
std::vector<recipe> recipeManager::searchByCategory(const std::string& category) {
    std::vector<recipe> recipes;

    try {
        auto collection = db["recipes"];
        bsoncxx::builder::stream::document filter{};
        filter << "category" << category;

        auto cursor = collection.find(filter.view());

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string recipeCategory = safeGetString(doc, "category");
                std::string type = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, recipeCategory, type);
                recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in category search: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error while searching by category: " << e.what() << std::endl;
        throw DatabaseError("Failed to search by category: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error while searching by category: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error while searching by category: " + std::string(e.what()));
    }

    return recipes;
}

std::vector<recipe> recipeManager::searchByType(const std::string& type) {
    std::vector<recipe> recipes;

    try {
        auto collection = db["recipes"];
        bsoncxx::builder::stream::document filter{};
        filter << "type" << type;

        auto cursor = collection.find(filter.view());

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string category = safeGetString(doc, "category");
                std::string recipeType = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, category, recipeType);
                recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in type search: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error while searching by type: " << e.what() << std::endl;
        throw DatabaseError("Failed to search by type: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error while searching by type: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error while searching by type: " + std::string(e.what()));
    }

    return recipes;
}

std::vector<recipe> recipeManager::searchByCategoryAndType(const std::string& category, const std::string& type) {
    std::vector<recipe> recipes;

    try {
        auto collection = db["recipes"];
        bsoncxx::builder::stream::document filter{};
        filter << "$and" << bsoncxx::builder::stream::open_array
               << bsoncxx::builder::stream::open_document
               << "category" << category
               << bsoncxx::builder::stream::close_document
               << bsoncxx::builder::stream::open_document
               << "type" << type
               << bsoncxx::builder::stream::close_document
               << bsoncxx::builder::stream::close_array;

        auto cursor = collection.find(filter.view());

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string recipeCategory = safeGetString(doc, "category");
                std::string recipeType = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, recipeCategory, recipeType);
                recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in category/type search: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error while searching by category and type: " << e.what() << std::endl;
        throw DatabaseError("Failed to search by category and type: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error while searching by category and type: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error while searching by category and type: " + std::string(e.what()));
    }

    return recipes;
}

recipeManager::PaginatedResult recipeManager::getRecipesPaginated(int page, int pageSize) {
    PaginatedResult result;
    result.page = page;
    result.pageSize = pageSize;

    try {
        auto collection = db["recipes"];

        // Get total count
        result.totalCount = collection.count_documents({});
        result.totalPages = (result.totalCount + pageSize - 1) / pageSize;

        // Validate page number
        if (page < 1) page = 1;
        if (page > result.totalPages && result.totalPages > 0) page = result.totalPages;

        // Calculate skip value
        int skip = (page - 1) * pageSize;

        // Get paginated results
        mongocxx::options::find options{};
        options.skip(skip);
        options.limit(pageSize);

        auto cursor = collection.find({}, options);

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string category = safeGetString(doc, "category");
                std::string type = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, category, type);
                result.recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in pagination: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error in pagination: " << e.what() << std::endl;
        throw DatabaseError("Failed to get paginated results: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in pagination: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error in pagination: " + std::string(e.what()));
    }

    return result;
}

recipeManager::PaginatedResult recipeManager::searchRecipesPaginated(const std::string& criteria, int page, int pageSize) {
    PaginatedResult result;
    result.page = page;
    result.pageSize = pageSize;

    try {
        auto collection = db["recipes"];

        // Create search filter
        bsoncxx::builder::stream::document filter{};
        if (!criteria.empty()) {
            filter << "title" << bsoncxx::builder::stream::open_document
                   << "$regex" << criteria
                   << "$options" << "i"
                   << bsoncxx::builder::stream::close_document;
        }

        // Get total count for search
        result.totalCount = collection.count_documents(filter.view());
        result.totalPages = (result.totalCount + pageSize - 1) / pageSize;

        // Validate page number
        if (page < 1) page = 1;
        if (page > result.totalPages && result.totalPages > 0) page = result.totalPages;

        // Calculate skip value
        int skip = (page - 1) * pageSize;

        // Get paginated search results
        mongocxx::options::find options{};
        options.skip(skip);
        options.limit(pageSize);

        auto cursor = collection.find(filter.view(), options);

        for (auto&& doc : cursor) {
            try {
                std::string title = safeGetString(doc, "title");
                std::string ingredients = safeGetString(doc, "ingredients");
                std::string instructions = safeGetString(doc, "instructions");
                std::string servingSize = safeGetString(doc, "servingSize");
                std::string cookTime = safeGetString(doc, "cookTime");
                std::string category = safeGetString(doc, "category");
                std::string type = safeGetString(doc, "type");

                recipe recipe(title, ingredients, instructions, servingSize, cookTime, category, type);
                result.recipes.push_back(recipe);
            } catch (const recipe::ValidationError& e) {
                std::cerr << "Warning: Skipping invalid recipe in search pagination: " << e.what() << std::endl;
                continue;
            }
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Database error in search pagination: " << e.what() << std::endl;
        throw DatabaseError("Failed to get paginated search results: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error in search pagination: " << e.what() << std::endl;
        throw DatabaseError("Unexpected error in search pagination: " + std::string(e.what()));
    }

    return result;
}

bool recipeManager::isConnected() {
    try {
        // Simple ping to test connection
        auto ping_cmd = bsoncxx::builder::stream::document{} << "ping" << 1 << bsoncxx::builder::stream::finalize;
        db.run_command(ping_cmd.view());
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string recipeManager::safeGetString(const bsoncxx::document::view& doc, const std::string& fieldName) {
    try {
        auto element = doc[fieldName];
        if (element && element.type() == bsoncxx::type::k_string) {
            return std::string(element.get_string().value);
        } else {
            throw DatabaseError("Field '" + fieldName + "' is missing or not a string");
        }
    } catch (const mongocxx::exception& e) {
        throw DatabaseError("Error reading field '" + fieldName + "': " + std::string(e.what()));
    }
}

// JSON response methods for web interface
std::string recipeManager::getRecipesJson() {
    try {
        auto recipes = viewRecipes();
        std::string jsonResponse = "{\"success\":true,\"recipes\":[";
        bool first = true;

        for (const auto& recipe : recipes) {
            if (!first) jsonResponse += ",";
            jsonResponse += recipe.toJson();
            first = false;
        }

        jsonResponse += "]}";
        return jsonResponse;
    } catch (const DatabaseError& e) {
        return "{\"success\":false,\"error\":\"" + std::string(e.what()) + "\"}";
    } catch (const std::exception& e) {
        return "{\"success\":false,\"error\":\"Unexpected error: " + std::string(e.what()) + "\"}";
    }
}

std::string recipeManager::searchRecipesJson(const std::string& criteria) {
    try {
        auto recipes = searchRecipes(criteria);
        std::string jsonResponse = "{\"success\":true,\"recipes\":[";
        bool first = true;

        for (const auto& recipe : recipes) {
            if (!first) jsonResponse += ",";
            jsonResponse += recipe.toJson();
            first = false;
        }

        jsonResponse += "]}";
        return jsonResponse;
    } catch (const DatabaseError& e) {
        return "{\"success\":false,\"error\":\"" + std::string(e.what()) + "\"}";
    } catch (const std::exception& e) {
        return "{\"success\":false,\"error\":\"Unexpected error: " + std::string(e.what()) + "\"}";
    }
}

std::string recipeManager::addRecipeJson(const std::string& jsonInput) {
    try {
        recipe newRecipe = recipe::fromJson(jsonInput);
        auto result = addRecipe(newRecipe);

        if (result.success) {
            return "{\"success\":true,\"message\":\"Recipe added successfully\"}";
        } else {
            return "{\"success\":false,\"error\":\"" + result.errorMessage + "\"}";
        }
    } catch (const recipe::ValidationError& e) {
        return "{\"success\":false,\"error\":\"Validation error: " + std::string(e.what()) + "\"}";
    } catch (const DatabaseError& e) {
        return "{\"success\":false,\"error\":\"Database error: " + std::string(e.what()) + "\"}";
    } catch (const std::exception& e) {
        return "{\"success\":false,\"error\":\"Unexpected error: " + std::string(e.what()) + "\"}";
    }
}

std::string recipeManager::updateRecipeJson(const std::string& title, const std::string& jsonInput) {
    try {
        recipe updatedRecipe = recipe::fromJson(jsonInput);
        auto result = updateRecipe(title, updatedRecipe);

        if (result.success) {
            return "{\"success\":true,\"message\":\"Recipe updated successfully\"}";
        } else {
            return "{\"success\":false,\"error\":\"" + result.errorMessage + "\"}";
        }
    } catch (const recipe::ValidationError& e) {
        return "{\"success\":false,\"error\":\"Validation error: " + std::string(e.what()) + "\"}";
    } catch (const DatabaseError& e) {
        return "{\"success\":false,\"error\":\"Database error: " + std::string(e.what()) + "\"}";
    } catch (const std::exception& e) {
        return "{\"success\":false,\"error\":\"Unexpected error: " + std::string(e.what()) + "\"}";
    }
}

std::string recipeManager::deleteRecipeJson(const std::string& title) {
    try {
        auto result = deleteRecipe(title);

        if (result.success) {
            return "{\"success\":true,\"message\":\"Recipe deleted successfully\"}";
        } else {
            return "{\"success\":false,\"error\":\"" + result.errorMessage + "\"}";
        }
    } catch (const DatabaseError& e) {
        return "{\"success\":false,\"error\":\"Database error: " + std::string(e.what()) + "\"}";
    } catch (const std::exception& e) {
        return "{\"success\":false,\"error\":\"Unexpected error: " + std::string(e.what()) + "\"}";
    }
}
