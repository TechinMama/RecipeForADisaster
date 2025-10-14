#include <gtest/gtest.h>
#include "recipe.h"

// Test fixture for Recipe tests
class RecipeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test Recipe constructor and getters
TEST_F(RecipeTest, ConstructorAndGetters) {
    recipe testRecipe("Chocolate Chip Cookies",
                     "2 cups flour, 1 cup butter, 1 cup sugar",
                     "Preheat oven, mix ingredients, bake",
                     "24 cookies",
                     "12-15 minutes",
                     "Dessert",
                     "Cookies");

    EXPECT_EQ(testRecipe.getTitle(), "Chocolate Chip Cookies");
    EXPECT_EQ(testRecipe.getIngredients(), "2 cups flour, 1 cup butter, 1 cup sugar");
    EXPECT_EQ(testRecipe.getInstructions(), "Preheat oven, mix ingredients, bake");
    EXPECT_EQ(testRecipe.getServingSize(), "24 cookies");
    EXPECT_EQ(testRecipe.getCookTime(), "12-15 minutes");
    EXPECT_EQ(testRecipe.getCategory(), "Dessert");
    EXPECT_EQ(testRecipe.getType(), "Cookies");
}

// Test Recipe setters
TEST_F(RecipeTest, Setters) {
    recipe testRecipe("Initial", "initial ingredients", "initial instructions", "4 servings", "30 min", "Main", "Test");

    testRecipe.setTitle("Test Recipe");
    testRecipe.setIngredients("Test ingredients");
    testRecipe.setInstructions("Test instructions");
    testRecipe.setServingSize("4 servings");
    testRecipe.setCookTime("30 minutes");
    testRecipe.setCategory("Main Course");
    testRecipe.setType("Pasta");

    EXPECT_EQ(testRecipe.getTitle(), "Test Recipe");
    EXPECT_EQ(testRecipe.getIngredients(), "Test ingredients");
    EXPECT_EQ(testRecipe.getInstructions(), "Test instructions");
    EXPECT_EQ(testRecipe.getServingSize(), "4 servings");
    EXPECT_EQ(testRecipe.getCookTime(), "30 minutes");
    EXPECT_EQ(testRecipe.getCategory(), "Main Course");
    EXPECT_EQ(testRecipe.getType(), "Pasta");
}

// Test Recipe JSON serialization
TEST_F(RecipeTest, JsonSerialization) {
    recipe originalRecipe("Test Recipe",
                         "ingredient1, ingredient2",
                         "step1, step2",
                         "4 servings",
                         "30 min",
                         "Main",
                         "Test");

    std::string jsonStr = originalRecipe.toJson();

    // Verify JSON contains expected fields
    EXPECT_NE(jsonStr.find("Test Recipe"), std::string::npos);
    EXPECT_NE(jsonStr.find("ingredient1"), std::string::npos);
    EXPECT_NE(jsonStr.find("step1"), std::string::npos);
}

// Test Recipe JSON deserialization
TEST_F(RecipeTest, JsonDeserialization) {
    std::string jsonStr = R"(
    {
        "title": "JSONRecipe",
        "ingredients": "flour,sugar,eggs",
        "instructions": "mix,bake,eat",
        "servingSize": "6servings",
        "cookTime": "45minutes",
        "category": "Dessert",
        "type": "Cake"
    })";

    recipe deserializedRecipe = recipe::fromJson(jsonStr);

    EXPECT_EQ(deserializedRecipe.getTitle(), "JSONRecipe");
    EXPECT_EQ(deserializedRecipe.getIngredients(), "flour,sugar,eggs");
    EXPECT_EQ(deserializedRecipe.getInstructions(), "mix,bake,eat");
    EXPECT_EQ(deserializedRecipe.getServingSize(), "6servings");
    EXPECT_EQ(deserializedRecipe.getCookTime(), "45minutes");
    EXPECT_EQ(deserializedRecipe.getCategory(), "Dessert");
    EXPECT_EQ(deserializedRecipe.getType(), "Cake");
}

// Test Recipe validation - empty title should throw
TEST_F(RecipeTest, ValidationEmptyTitle) {
    EXPECT_THROW({
        recipe invalidRecipe("", "ingredients", "instructions", "servings", "time", "category", "type");
    }, recipe::ValidationError);
}

// Test Recipe validation - empty ingredients should throw
TEST_F(RecipeTest, ValidationEmptyIngredients) {
    EXPECT_THROW({
        recipe invalidRecipe("Title", "", "instructions", "servings", "time", "category", "type");
    }, recipe::ValidationError);
}

// Test Recipe validation - empty instructions should throw
TEST_F(RecipeTest, ValidationEmptyInstructions) {
    EXPECT_THROW({
        recipe invalidRecipe("Title", "ingredients", "", "servings", "time", "category", "type");
    }, recipe::ValidationError);
}