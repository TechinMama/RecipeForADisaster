#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include "recipe.h"
#include <vector>
#include <string>

class recipeManager {
public:
    void addRecipe(const recipe& recipe);
    void deleteRecipe(const std::string& name);
    recipe* findRecipe(const std::string& name);
    void loadRecipes(const std::string& filename);
    void saveRecipes(const std::string& filename) const;
    const std::vector<recipe>& getAllRecipes() const;

private:
    std::vector<recipe> recipes;
};

#endif
