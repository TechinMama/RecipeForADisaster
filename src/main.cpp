#include <iostream>
#include "recipe.h"
#include "recipeManager.h"
#include <sstream>

void displayMenu() {
    std::cout << "1. Add Recipe\n";
    std::cout << "2. View Recipes\n";
    std::cout << "3. Delete Recipe\n";
    std::cout << "4. Exit\n";
}

int main() {
    recipeManager recipeManager;
    // Load existing recipes
    // recipeManager.loadRecipes("recipes.json");

    int choice;
    while (true) {
        displayMenu();
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: {
                // Logic for adding a recipe
                std::string name, category, instructions;
                int cooktime;
                std::vector<std::string> ingredients;

                std::cout << "Enter recipe name: ";
                std::cin.ignore();
                std::getline(std::cin, name);

                std::cout << "Enter ingredients (comma-separated): ";
                std::string ing;
                std::getline(std::cin, ing);
                std::stringstream ss(ing);
                std::string ingredient;
                while (std::getline(ss, ingredient, ',')) {
                    ingredients.push_back(ingredient);
                }

                std::cout << "Enter instructions: ";
                std::getline(std::cin, instructions);

                std::cout << "Enter cooking time (minutes): ";
                std::cin >> cooktime;

                std::cout << "Enter category: ";
                std::cin.ignore();
                std::getline(std::cin, category);

                recipe newRecipe(name, ingredients, instructions, cooktime, category);
                recipeManager.addRecipe(newRecipe);
                break;
            }
            case 2: {
                // Logic for viewing recipes
                const auto& recipes = recipeManager.getAllRecipes();
                for (const auto& recipe : recipes) {
                    std::cout << "Name: " << recipe.getName() << "\n";
                }
                break;
            }
            case 3: {
                // Logic for deleting a recipe
                std::string name;
                std::cout << "Enter recipe name to delete: ";
                std::cin.ignore();
                std::getline(std::cin, name);
                recipeManager.deleteRecipe(name);
                break;
            }
            case 4:
                // Save recipes and exit
                // recipeManager.saveRecipes("recipes.json");
                return 0;
            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    }
}
