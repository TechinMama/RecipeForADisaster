#include "recipeManager.h"
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

recipeManager::recipeManager(const std::string& uri) 
    : client(mongocxx::uri{uri}), db(client["RecipeManagerDB"]) {}  

Specify your database name

void recipeManager::addRecipe(const Recipe& recipe) {
    auto collection = db["recipes"];
    bsoncxx::builder::stream::document document{};
    document << "title" << recipe.getTitle()
             << "ingredients" << recipe.getIngredients()
             << "instructions" << recipe.getInstructions()
             << "servingSize" << recipe.getServingSize()
             << "cookTime" << recipe.getCookTime()
             << "cuisine" << recipe.getCuisine()
             << "category" << recipe.getCategory();
    collection.insert_one(document.view());
}


std::vector<recipe> recipeManager::viewRecipes() {
    auto collection = db["recipes"];
    std::vector<recipe> recipes;
    auto cursor = collection.find({});
    
    for (auto&& doc : cursor) {
        Recipe recipe(
            doc["title"].get_utf8().value.to_string(),
            doc["ingredients"].get_utf8().value.to_string(),
            doc["instructions"].get_utf8().value.to_string(),
            doc["servingSize"].get_utf8().value.to_string(),
            

doc["cookTime"].get_utf8().value.to_string(),
            doc["cuisine"].get_utf8().value.to_string(),
            doc["category"].get_utf8().value.to_string()
        );
        recipes.push_back(recipe);
    }
    return recipes;
}

bool recipeManager::updateRecipe(const std::string& title, const Recipe& updatedRecipe) {
    auto collection = db["recipes"];
    bsoncxx::builder::stream::document filter{};
    filter << "title" << title;

    bsoncxx::builder::stream::document 

update{};
    update << "$set" << bsoncxx::builder::stream::open_document
           << "ingredients" << updatedRecipe.getIngredients()
           << "instructions" << updatedRecipe.getInstructions()
           << "servingSize" << updatedRecipe.getServingSize()
           << "cookTime" << updatedRecipe.getCookTime()
           << "cuisine" << updatedRecipe.getCuisine()
           << "category" << updatedRecipe.getCategory()
           << bsoncxx::builder::stream::close_document;

    auto result = collection.update_one(filter.view(), update.view());

    return result ? result->modified_count() > 0 : false;
}

bool recipeManager::deleteRecipe(const std::string& title) {
    auto collection = db["recipes"];
    bsoncxx::builder::stream::document filter{};
    filter << "title" << title;

    auto result = collection.delete_one(filter.view());
    return result ? result->deleted_count() > 0 : false;
}

std::vector<recipe> recipeManager::searchRecipes(const std::string& criteria) {
    auto collection = db["recipes"];

    std::vector<recipe> recipes;
    bsoncxx::builder::stream::document filter{};
    filter << "title" << bsoncxx::builder::stream::open_document
           << "$regex" << criteria
           << "$options" << "i" // case insensitive
           << bsoncxx::builder::stream::close_document;

    auto cursor = collection.find(filter.view());
    for (auto&& doc : cursor) {
        recipe recipe(
            doc["title"].get_utf8().value.to_string(),
            doc["ingredients"].get_utf8().value.to_string(),
            doc["instructions"].get_utf8().value.to_string(),

            doc["servingSize"].get_utf8().value.to_string(),
            doc["cookTime"].get_utf8().value.to_string(),
            doc["cuisine"].get_utf8().value.to_string(),
            doc["category"].get_utf8().value.to_string()
        );
        recipes.push_back(recipe);
    }
    return recipes;
}
