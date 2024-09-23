#include "Recipe.h"

Recipe::Recipe(const std::string& name, const std::vector<std::string>& ingredients, 
               const std::string& instructions, int cooktime, const std::string& category)
    : name(name), ingredients(ingredients), instructions(instructions), 
      cooktime(cooktime), category(category) {}

std::string Recipe::getName() const { return name; }
std::vector<std::string> Recipe::getIngredients() const { return ingredients; }
std::string Recipe::getInstructions() const { return instructions; }
int Recipe::getcooktime() const { return cooktime; }
std::string Recipe::getCategory() const { return category; }
