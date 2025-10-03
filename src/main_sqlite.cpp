#include <iostream>
#include "recipeManagerSQLite.h"
#include "recipe.h"

int main() {
    try {
        // Use SQLite instead of MongoDB
        RecipeManagerSQLite manager("recipes.db");

        if (!manager.isConnected()) {
            std::cerr << "Error: Failed to connect to SQLite database" << std::endl;
            return 1;
        }

        std::cout << "Connected to SQLite database successfully!" << std::endl;

        // Example recipe usage with error handling
        try {
            recipe newRecipe("Pasta Carbonara",
                           "400g spaghetti, 200g pancetta, 2 large eggs, 100g grated Pecorino Romano, Black pepper",
                           "Cook spaghetti in salted boiling water. Fry pancetta until crispy. Whisk eggs with cheese and pepper. Combine hot pasta with pancetta. Add egg mixture off heat, tossing quickly",
                           "4 servings",
                           "20 minutes",
                           "Italian",
                           "Main Course");

            if (manager.addRecipe(newRecipe)) {
                std::cout << "Recipe added successfully!" << std::endl;
            } else {
                std::cout << "Failed to add recipe" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Recipe creation failed: " << e.what() << std::endl;
        }

        // View recipes with error handling
        try {
            auto recipes = manager.getAllRecipes();
            std::cout << "\nFound " << recipes.size() << " recipes:" << std::endl;

            for (const auto& recipe : recipes) {
                std::cout << "Title: " << recipe.getTitle() << "\n"
                          << "Ingredients: " << recipe.getIngredients() << "\n"
                          << "Instructions: " << recipe.getInstructions() << "\n"
                          << "Serving Size: " << recipe.getServingSize() << "\n"
                          << "Cook Time: " << recipe.getCookTime() << "\n"
                          << "Category: " << recipe.getCategory() << "\n"
                          << "Type: " << recipe.getType() << "\n\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Failed to retrieve recipes: " << e.what() << std::endl;
        }

        // Search example
        try {
            auto pastaRecipes = manager.searchByTitle("Pasta");
            std::cout << "Found " << pastaRecipes.size() << " pasta recipes" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Search failed: " << e.what() << std::endl;
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }
}