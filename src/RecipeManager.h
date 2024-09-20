#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include "Recipe.h"
#include <vector>
#include <string>

class RecipeManager {
public:
    void addRecipe(const Recipe& recipe);
    void deleteRecipe(const std::string& name);
    Recipe* findRecipe(const std::string& name);
    void loadRecipes(const std::string& filename);
    void saveRecipes(const std::string& filename) const;
    const std::vector<Recipe>& getAllRecipes() const;

private:
    std::vector<Recipe> recipes;
};

#endif
