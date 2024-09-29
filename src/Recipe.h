#ifndef RECIPE_H
#define RECIPE_H

#include <string>


class recipe {
public:
    recipe(const std::string& title, const std::string& ingredients, const std::string& instructions,
           const std::string& servingSize, const std::string& cookTime,
           const std::string& category, const std::string& type, const std::string& id = "");

    std::string getId() const;
    std::string getTitle() const;
    std::string getIngredients() const;
    std::string getInstructions() const;
    std::string getServingSize() const;
    std::string getCookTime() const;
    std::string getCategory() const;
    std::string getType() const;

    void setTitle(const std::string& title);

    void setIngredients(const std::string& ingredients);
    void setInstructions(const std::string& instructions);
    void setServingSize(const std::string& servingSize);
    void setCookTime(const std::string& cookTime);
    void setCategory(const std::string& category);
    void setType(const std::string& type);

private:
    std::string id;
    std::string title;
    std::string ingredients;
    std::string instructions;
    std::string servingSize;
    std::string cookTime;
    std::string category;
    std::string type;

};

#endif // RECIPE_H

