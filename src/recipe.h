#ifndef RECIPE_H
#define RECIPE_H

#include <string>
#include <stdexcept>


class recipe {
public:
    // Custom exception for validation errors
    class ValidationError : public std::runtime_error {
    public:
        explicit ValidationError(const std::string& message) : std::runtime_error(message) {}
    };

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

    // JSON serialization methods
    std::string toJson() const;
    static recipe fromJson(const std::string& jsonStr);

private:
    std::string id;
    std::string title;
    std::string ingredients;
    std::string instructions;
    std::string servingSize;
    std::string cookTime;
    std::string category;
    std::string type;

    // Validation helper methods
    void validateTitle(const std::string& title) const;
    void validateIngredients(const std::string& ingredients) const;
    void validateInstructions(const std::string& instructions) const;
    void validateServingSize(const std::string& servingSize) const;
    void validateCookTime(const std::string& cookTime) const;
    void validateCategory(const std::string& category) const;
    void validateType(const std::string& type) const;

    // JSON helper methods
    std::string escapeJsonString(const std::string& str) const;
    static std::string unescapeJsonString(const std::string& str);
};

#endif // RECIPE_H

