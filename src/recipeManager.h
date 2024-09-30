#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <vector>
#include <string>
#include "recipe.h"

class recipeManager {
public:
    recipeManager(const std::string& uri);
    void addRecipe(const recipe& recipe);
    std::vector<recipe> viewRecipes();
    bool updateRecipe(const std::string& 

title, const recipe& updatedRecipe);
    bool deleteRecipe(const std::string& title);
    std::vector<recipe> searchRecipes(const std::string& criteria);

private:
    mongocxx::client client;
    mongocxx::database db;
};

#endif // RECIPE_MANAGER_H
