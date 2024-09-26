#include "RecipeManager.h"
#include <fstream>
#include <iostream>

void RecipeManager::addRecipe(const Recipe& recipe) {
    recipes.push_back(recipe);
}

void RecipeManager::deleteRecipe(const std::string& name) {
    recipes.erase(std::remove_if(recipes.begin(), recipes.end(),
                                  [&name](const Recipe& r) { return r.getName() == name; }), recipes.end());
}

Recipe* RecipeManager::findRecipe(const std::string& name) {
    for (auto& recipe : recipes) {
        if (recipe.getName() == name) {
            return &recipe;
        }
    }
    return nullptr;
}

void RecipeManager::loadRecipes(const std::string& filename) {
    // Implementation for loading from a file (CSV or JSON)
}

void RecipeManager::saveRecipes(const std::string& filename) const {
    // Implementation for saving to a file (CSV or JSON)
}

const std::vector<Recipe>& RecipeManager::getAllRecipes() const {
    return recipes;
}
