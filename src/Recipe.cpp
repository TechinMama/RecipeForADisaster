#include "recipe.h"

recipe::recipe(const std::string& name, const std::vector<std::string>& ingredients, 
               const std::string& instructions, int cooktime, const std::string& category)
    : name(name), ingredients(ingredients), instructions(instructions), 
      cooktime(cooktime), category(category) {}

std::string recipe::getName() const { return name; }
std::vector<std::string> recipe::getIngredients() const { return ingredients; }
std::string recipe::getInstructions() const { return instructions; }
int recipe::getcooktime() const { return cooktime; }
std::string recipe::getCategory() const { return category; }
