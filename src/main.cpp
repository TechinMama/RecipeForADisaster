#include <iostream>
#include "recipeManager.h"

int main() {
    mongocxx::instance instance{};
    recipeManager manager("your_mongodb_uri"); // Replace with your hosted MongoDB URI

    // Example usage:
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

    // Further operations (update, delete, search) can be implemented similarly.

    return 0;
}
