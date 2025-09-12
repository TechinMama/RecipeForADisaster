#include <iostream>
#include "recipeManager.h"

int main() {
    try {
        mongocxx::instance instance{};

        // Read MongoDB URI from environment variable
        const char* mongoUri = std::getenv("MONGODB_URI");
        if (!mongoUri) {
            std::cerr << "Error: MONGODB_URI environment variable not set" << std::endl;
            std::cerr << "Please set it with: export MONGODB_URI=\"mongodb://localhost:27017\"" << std::endl;
            return 1;
        }

        recipeManager manager(mongoUri);

        std::cout << "Connected to database successfully!" << std::endl;

        // Example recipe usage with error handling
        try {
            recipe newRecipe("Pasta", "Pasta, Tomato Sauce", "Boil pasta, add sauce",
                           "4 servings", "20 minutes", "Italian", "Main Course");

            auto result = manager.addRecipe(newRecipe);
            if (result.success) {
                std::cout << "Recipe added successfully!" << std::endl;
            } else {
                std::cout << "Failed to add recipe: " << result.errorMessage << std::endl;
            }
        } catch (const recipe::ValidationError& e) {
            std::cout << "Recipe validation failed: " << e.what() << std::endl;
        }

        // View recipes with error handling
        try {
            auto recipes = manager.viewRecipes();
            std::cout << "\nFound " << recipes.size() << " recipes:" << std::endl;

            for (const auto& recipe : recipes) {
                std::cout << "Title: " << recipe.getTitle() << "\n"
                          << "Serving Size: " << recipe.getServingSize() << "\n"
                          << "Cook Time: " << recipe.getCookTime() << "\n"
                          << "Category: " << recipe.getCategory() << "\n"
                          << "Type: " << recipe.getType() << "\n\n";
            }
        } catch (const recipeManager::DatabaseError& e) {
            std::cout << "Failed to retrieve recipes: " << e.what() << std::endl;
        }

        // Update a recipe with error handling
        try {
            recipe updatedRecipe("Pasta", "Pasta, Tomato Sauce, Garlic", "Boil pasta, add sauce, add garlic",
                               "4 servings", "25 minutes", "Italian", "Main Course");

            auto result = manager.updateRecipe("Pasta", updatedRecipe);
            if (result.success) {
                std::cout << "Recipe updated successfully!" << std::endl;
            } else {
                std::cout << "Failed to update recipe: " << result.errorMessage << std::endl;
            }
        } catch (const recipe::ValidationError& e) {
            std::cout << "Updated recipe validation failed: " << e.what() << std::endl;
        }

        // Delete a recipe with error handling
        auto deleteResult = manager.deleteRecipe("Pasta");
        if (deleteResult.success) {
            std::cout << "Recipe deleted successfully!" << std::endl;
        } else {
            std::cout << "Failed to delete recipe: " << deleteResult.errorMessage << std::endl;
        }

        // Search for recipes with error handling
        try {
            auto searchResults = manager.searchRecipes("Pasta");
            std::cout << "\nSearch results: " << searchResults.size() << " recipes found" << std::endl;

            for (const auto& recipe : searchResults) {
                std::cout << "Found Recipe: " << recipe.getTitle() << std::endl;
            }
        } catch (const recipeManager::DatabaseError& e) {
            std::cout << "Search failed: " << e.what() << std::endl;
        }

    } catch (const recipeManager::DatabaseError& e) {
        std::cout << "Database connection failed: " << e.what() << std::endl;
        std::cout << "Please check your MongoDB URI and connection." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
