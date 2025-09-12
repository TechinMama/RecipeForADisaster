#include "recipe.h"
#include <algorithm>
#include <cctype>

recipe::recipe(const std::string& title, const std::string& ingredients, const std::string& instructions,
               const std::string& servingSize, const std::string& cookTime,
               const std::string& category, const std::string& type, const std::string& id)
{
    // Validate all input parameters
    validateTitle(title);
    validateIngredients(ingredients);
    validateInstructions(instructions);
    validateServingSize(servingSize);
    validateCookTime(cookTime);
    validateCategory(category);
    validateType(type);

    // Assign validated values
    this->title = title;
    this->ingredients = ingredients;
    this->instructions = instructions;
    this->servingSize = servingSize;
    this->cookTime = cookTime;
    this->category = category;
    this->type = type;
    this->id = id;
}

std::string recipe::getId() const { return id; }
std::string recipe::getTitle() const { return title; }
std::string recipe::getIngredients() const { return ingredients; }
std::string recipe::getInstructions() const { return instructions; }
std::string recipe::getServingSize() const { return servingSize; }
std::string recipe::getCookTime() const { return cookTime; }
std::string recipe::getCategory() const { return category; }
std::string recipe::getType() const { return type; }

void recipe::setTitle(const std::string& title) {
    validateTitle(title);
    this->title = title;
}

void recipe::setIngredients(const std::string& ingredients) {
    validateIngredients(ingredients);
    this->ingredients = ingredients;
}

void recipe::setInstructions(const std::string& instructions) {
    validateInstructions(instructions);
    this->instructions = instructions;
}

void recipe::setServingSize(const std::string& servingSize) {
    validateServingSize(servingSize);
    this->servingSize = servingSize;
}

void recipe::setCookTime(const std::string& cookTime) {
    validateCookTime(cookTime);
    this->cookTime = cookTime;
}

void recipe::setCategory(const std::string& category) {
    validateCategory(category);
    this->category = category;
}

void recipe::setType(const std::string& type) {
    validateType(type);
    this->type = type;
}

// Validation helper methods implementation
void recipe::validateTitle(const std::string& title) const {
    if (title.empty()) {
        throw ValidationError("Recipe title cannot be empty");
    }
    if (title.length() > 100) {
        throw ValidationError("Recipe title cannot exceed 100 characters");
    }
    // Check for only whitespace
    if (title.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        throw ValidationError("Recipe title cannot contain only whitespace");
    }
}

void recipe::validateIngredients(const std::string& ingredients) const {
    if (ingredients.empty()) {
        throw ValidationError("Recipe ingredients cannot be empty");
    }
    if (ingredients.length() > 1000) {
        throw ValidationError("Recipe ingredients cannot exceed 1000 characters");
    }
    // Check for only whitespace
    if (ingredients.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        throw ValidationError("Recipe ingredients cannot contain only whitespace");
    }
}

void recipe::validateInstructions(const std::string& instructions) const {
    if (instructions.empty()) {
        throw ValidationError("Recipe instructions cannot be empty");
    }
    if (instructions.length() > 2000) {
        throw ValidationError("Recipe instructions cannot exceed 2000 characters");
    }
    // Check for only whitespace
    if (instructions.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        throw ValidationError("Recipe instructions cannot contain only whitespace");
    }
}

void recipe::validateServingSize(const std::string& servingSize) const {
    if (servingSize.empty()) {
        throw ValidationError("Recipe serving size cannot be empty");
    }
    if (servingSize.length() > 50) {
        throw ValidationError("Recipe serving size cannot exceed 50 characters");
    }
}

void recipe::validateCookTime(const std::string& cookTime) const {
    if (cookTime.empty()) {
        throw ValidationError("Recipe cook time cannot be empty");
    }
    if (cookTime.length() > 50) {
        throw ValidationError("Recipe cook time cannot exceed 50 characters");
    }
}

void recipe::validateCategory(const std::string& category) const {
    if (category.empty()) {
        throw ValidationError("Recipe category cannot be empty");
    }
    if (category.length() > 50) {
        throw ValidationError("Recipe category cannot exceed 50 characters");
    }
}

void recipe::validateType(const std::string& type) const {
    if (type.empty()) {
        throw ValidationError("Recipe type cannot be empty");
    }
    if (type.length() > 50) {
        throw ValidationError("Recipe type cannot exceed 50 characters");
    }
}

// JSON serialization implementation
std::string recipe::toJson() const {
    return "{"
           "\"id\":\"" + escapeJsonString(id) + "\","
           "\"title\":\"" + escapeJsonString(title) + "\","
           "\"ingredients\":\"" + escapeJsonString(ingredients) + "\","
           "\"instructions\":\"" + escapeJsonString(instructions) + "\","
           "\"servingSize\":\"" + escapeJsonString(servingSize) + "\","
           "\"cookTime\":\"" + escapeJsonString(cookTime) + "\","
           "\"category\":\"" + escapeJsonString(category) + "\","
           "\"type\":\"" + escapeJsonString(type) + "\""
           "}";
}

recipe recipe::fromJson(const std::string& jsonStr) {
    std::string cleanedJson = jsonStr;

    // Remove whitespace
    cleanedJson.erase(std::remove_if(cleanedJson.begin(), cleanedJson.end(),
        [](char c) { return std::isspace(c); }), cleanedJson.end());

    // Basic JSON validation
    if (cleanedJson.empty() || cleanedJson[0] != '{' || cleanedJson.back() != '}') {
        throw ValidationError("Invalid JSON format");
    }

    // Extract field values (simple parsing - for production, consider using a proper JSON library)
    std::string id, title, ingredients, instructions, servingSize, cookTime, category, type;

    // Helper function to extract string value
    auto extractString = [](const std::string& json, const std::string& field) -> std::string {
        std::string searchStr = "\"" + field + "\":\"";
        size_t startPos = json.find(searchStr);
        if (startPos == std::string::npos) return "";

        startPos += searchStr.length();
        size_t endPos = json.find("\"", startPos);
        if (endPos == std::string::npos) return "";

        std::string value = json.substr(startPos, endPos - startPos);
        return unescapeJsonString(value);
    };

    id = extractString(cleanedJson, "id");
    title = extractString(cleanedJson, "title");
    ingredients = extractString(cleanedJson, "ingredients");
    instructions = extractString(cleanedJson, "instructions");
    servingSize = extractString(cleanedJson, "servingSize");
    cookTime = extractString(cleanedJson, "cookTime");
    category = extractString(cleanedJson, "category");
    type = extractString(cleanedJson, "type");

    return recipe(title, ingredients, instructions, servingSize, cookTime, category, type, id);
}

std::string recipe::escapeJsonString(const std::string& str) const {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string recipe::unescapeJsonString(const std::string& str) {
    std::string unescaped;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': unescaped += '"'; ++i; break;
                case '\\': unescaped += '\\'; ++i; break;
                case 'n': unescaped += '\n'; ++i; break;
                case 'r': unescaped += '\r'; ++i; break;
                case 't': unescaped += '\t'; ++i; break;
                default: unescaped += str[i]; break;
            }
        } else {
            unescaped += str[i];
        }
    }
    return unescaped;
}
