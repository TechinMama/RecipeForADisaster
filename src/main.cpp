#include <iostream>
#include "recipeManager.h"

int main() {
    mongocxx::instance instance{};
    recipeManager manager("your_mongodb_uri"); // Replace with your hosted MongoDB URI

    // Example recipe usage:
    recipe newRecipe("Pasta", "Pasta, Tomato Sauce", "Boil pasta, add sauce", 
                     "4 servings", "20 minutes", "Italian", "Main Course");
    manager.addRecipe(newRecipe);

    auto recipes = manager.viewRecipes();
    for (const auto& recipe : recipes) {

        std::cout << "Title: " << recipe.getTitle() << "\n"
                  << "Serving Size: " << recipe.getServingSize() << "\n"
                  << "Cook Time: " << recipe.getCookTime() << "\n"
                  << "Category: " << recipe.getCategory() << "\n"
                  << "Type: " << recipe.getType() << "\n\n";
    }

    // Update a recipe
    Recipe updatedRecipe("Pasta", "Pasta, Tomato Sauce, Garlic", "Boil pasta, add sauce, add garlic", 
                         "4 servings", "25 minutes", "Italian", "Main Course");
    if (manager.updateRecipe("Pasta", updatedRecipe)) {
        std::cout << "Recipe updated successfully.\n";
    } else {
        std::cout << "Recipe not found or update failed.\n";
    }

    // Delete a recipe
    if (manager.deleteRecipe("Pasta")) {
        std::cout << "Recipe deleted successfully.\n";
    } else {
        std::cout << "Recipe not found or deletion failed.\n";
    }

    // Search for a recipe
    auto searchResults = manager.searchRecipes("Pasta");
    for (const auto& recipe : searchResults) {
        std::cout << "Found Recipe: " << recipe.getTitle() << "\n";
    }

    return 0;
}
