#include "recipeManager.h"
#include <fstream>
#include <iostream>
#include "recipe.h"

void recipeManager::addRecipe(const recipe& recipe) {
    recipes.push_back(recipe);
}

void recipeManager::deleteRecipe(const std::string& name) {
    recipes.erase(std::remove_if(recipes.begin(), recipes.end(),
                                  [&name](const recipe& r) { return r.getName() == name; }), recipes.end());
}

recipe* recipeManager::findRecipe(const std::string& name) {
    for (auto& recipe : recipes) {
        if (recipe.getName() == name) {
            return &recipe;
        }
    }
    return nullptr;
}

void recipeManager::loadRecipes(const std::string& filename) {
    // Implementation for loading from a file (CSV or JSON)
}

void recipeManager::saveRecipes(const std::string& filename) const {
    // Implementation for saving to a file (CSV or JSON)
}

const std::vector<recipe>& recipeManager::getAllRecipes() const {
    return recipes;
}
