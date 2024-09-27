#ifndef RECIPE_H
#define RECIPE_H

#include <string>
#include <vector>

class recipe {
public:
    recipe(const std::string& name, const std::vector<std::string>& ingredients, 
           const std::string& instructions, int cooktime, const std::string& category);

    std::string getName() const;
    std::vector<std::string> getIngredients() const;
    std::string getInstructions() const;
    int getcooktime() const;
    std::string getCategory() const;

private:
    std::string name;
    std::vector<std::string> ingredients;
    std::string instructions;
    int cooktime; // in minutes
    std::string category;
};

#endif
