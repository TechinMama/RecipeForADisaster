#ifndef RECIPE_MANAGER_H
#define RECIPE_MANAGER_H

#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/client-fwd.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#include "recipe.h"
#include <vector>

class recipeManager {
public:
    recipeManager(const std::string& uri);
    void addRecipe(const recipe& recipe);
    std::vector<recipe> viewRecipes();
    void updateRecipe(const recipe& recipe);
    void deleteRecipe(const std::string& id);
    std::vector<recipe> searchRecipes(const std::string& query);

private:
    mongocxx::client client;
    mongocxx::database db;
};

#endif // RECIPE_MANAGER_H
