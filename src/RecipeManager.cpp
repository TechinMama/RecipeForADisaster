#include "recipeManager.h"
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

recipeManager::recipeManager(const std::string& uri) : client(mongocxx::uri{uri}), db(client["recipe_db"]) {}

void recipeManager::addRecipe(const Recipe& recipe) {
    auto collection = db["recipes"];
    bsoncxx::builder::stream::document document{};
    document << "title" << recipe.getTitle()
             << "ingredients" << recipe.getIngredients()
             << "instructions" << 

recipe.getInstructions()
             << "servingSize" << recipe.getServingSize()
             << "cookTime" << recipe.getCookTime()
             << "category" << recipe.getCategory()
             << "type" << recipe.getType();
    collection.insert_one(document.view());
}

std::vector<recipe> recipeManager::viewRecipes() {
    std::vector<recipe> recipes;
    auto collection = db["recipes"];
    auto cursor = collection.find({});
    
    for (const auto& doc : cursor) {
        recipes.emplace_back(
            bsoncxx::to_json(doc["_id"].get_oid().value).

to_string(),
            doc["title"].get_utf8().value.to_string(),
            doc["ingredients"].get_utf8().value.to_string(),
            doc["instructions"].get_utf8().value.to_string(),
            doc["servingSize"].get_utf8().value.to_string(),
            doc["cookTime"].get_utf8().value.to_string(),
            doc["category"].get_utf8().value.to_string(),
            doc["type"].get_utf8().value.to_string()
        );
    }

    return recipes;
}

void recipeManager::updateRecipe(const recipe& recipe) {
    auto collection = db["recipes"];
    bsoncxx::builder::stream::document filter_builder{};
    filter_builder << "_id" << bsoncxx::oid(recipe.getId());
    
    bsoncxx::builder::stream::document update_builder{};
    update_builder << "$set" << bsoncxx::builder::stream::open_document
                   << "title" << recipe.getTitle()
                   << "ingredients" << recipe.getIngredients()
                   << "instructions" << recipe.getInstructions()
                   << "servingSize" << 

recipe.getServingSize()
                   << "cookTime" << recipe.getCookTime()
                   << "category" << recipe.getCategory()
                   << "type" << recipe.getType()
                   << bsoncxx::builder::stream::close_document;
    
    collection.update_one(filter_builder.view(), update_builder.view());
}

void recipeManager::deleteRecipe(const std::string& id) {
    auto collection = db["recipes"];
    bsoncxx::builder::stream::document filter_builder{};
    filter_builder << "_id" << bsoncxx::oid{id};
    

collection.delete_one(filter_builder.view());
}

std::vector<recipe> recipeManager::searchRecipes(const std::string& query) {
    std::vector<recipe> recipes;
    auto collection = db["recipes"];
    auto cursor = collection.find(bsoncxx::builder::stream::document{} << "title" << bsoncxx::builder::stream::open_document << "$regex" << query << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize);
    
    for (const auto& doc : cursor) {
        recipes.emplace_back(
            bsoncxx::to_json(doc["_id"].get_oid().value).to_string(),

            doc["title"].get_utf8().value.to_string(),
            doc["ingredients"].get_utf8().value.to_string(),
            doc["instructions"].get_utf8().value.to_string(),
            doc["servingSize"].get_utf8().value.to_string(),
            doc["cookTime"].get_utf8().value.to_string(),
            doc["category"].get_utf8().value.to_string(),
            doc["type"].get_utf8().value.to_string()
        );
    }
    return recipes;

}

