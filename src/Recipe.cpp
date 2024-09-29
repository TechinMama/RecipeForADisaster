#include "recipe.h"

recipe::recipe(const std::string& title, const std::string& ingredients, const std::string& instructions,
               const std::string& servingSize, const std::string& cookTime,
               const std::string& category, const std::string& type, const std::string& id)
    : title(title), ingredients(ingredients), instructions(instructions),
      servingSize(servingSize), cookTime(cookTime), category(category), type(type), id(id) {}

std::string recipe::getId() const { return id; }

std::string recipe::getTitle() const { return title; }
std::string recipe::getIngredients() const { return ingredients; }
std::string recipe::getInstructions() const { return instructions; }
std::string recipe::getServingSize() const { return servingSize; }
std::string recipe::getCookTime() const { return cookTime; }
std::string recipe::getCategory() const { return category; }
std::string recipe::getType() const { return type; }

void recipe::setTitle(const std::string& title) { this->title = title; }
void recipe::setIngredients(const std::string& ingredients) { this->ingredients = ingredients; }
void recipe::setInstructions(const 

std::string& instructions) { this->instructions = instructions; }
void recipe::setServingSize(const std::string& servingSize) { this->servingSize = servingSize; }
void recipe::setCookTime(const std::string& cookTime) { this->cookTime = cookTime; }
void recipe::setCategory(const std::string& category) { this->category = category; }
void recipe::setType(const std::string& type) { this->type = type; }
