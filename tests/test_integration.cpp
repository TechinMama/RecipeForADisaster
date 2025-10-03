#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "recipe.h"
#include "recipeManagerSQLite.h"

// Test counters
int testsRun = 0;
int testsPassed = 0;
int testsFailed = 0;

// Helper function to run a test
void runTest(const std::string& testName, std::function<bool()> testFunc) {
    testsRun++;
    std::cout << "Running test: " << testName << "... ";

    try {
        if (testFunc()) {
            testsPassed++;
            std::cout << "PASSED" << std::endl;
        } else {
            testsFailed++;
            std::cout << "FAILED" << std::endl;
        }
    } catch (const std::exception& e) {
        testsFailed++;
        std::cout << "FAILED - Exception: " << e.what() << std::endl;
    }
}

// Test recipe validation
bool testRecipeValidation() {
    // Test valid recipe creation
    try {
        recipe validRecipe("Pasta Carbonara", "Pasta, Eggs, Cheese, Bacon",
                          "Cook pasta, mix with eggs and cheese, add bacon",
                          "4 servings", "20 minutes", "Italian", "Main Course");
        assert(validRecipe.getTitle() == "Pasta Carbonara");
        assert(validRecipe.getIngredients() == "Pasta, Eggs, Cheese, Bacon");
    } catch (const std::exception& e) {
        std::cerr << "Valid recipe creation failed: " << e.what() << std::endl;
        return false;
    }

    // Test empty title validation
    try {
        recipe("", "Ingredients", "Instructions", "4 servings", "20 min", "Italian", "Main");
        return false; // Should have thrown exception
    } catch (const recipe::ValidationError&) {
        // Expected
    }

    // Test whitespace-only title validation
    try {
        recipe("   ", "Ingredients", "Instructions", "4 servings", "20 min", "Italian", "Main");
        return false; // Should have thrown exception
    } catch (const recipe::ValidationError&) {
        // Expected
    }

    // Test title too long
    try {
        std::string longTitle(101, 'A'); // 101 characters
        recipe(longTitle, "Ingredients", "Instructions", "4 servings", "20 min", "Italian", "Main");
        return false; // Should have thrown exception
    } catch (const recipe::ValidationError&) {
        // Expected
    }

    // Test empty ingredients validation
    try {
        recipe("Title", "", "Instructions", "4 servings", "20 min", "Italian", "Main");
        return false; // Should have thrown exception
    } catch (const recipe::ValidationError&) {
        // Expected
    }

    // Test empty instructions validation
    try {
        recipe("Title", "Ingredients", "", "4 servings", "20 min", "Italian", "Main");
        return false; // Should have thrown exception
    } catch (const recipe::ValidationError&) {
        // Expected
    }

    // Test setter validation
    try {
        recipe testRecipe("Valid Title", "Valid Ingredients", "Valid Instructions",
                         "4 servings", "20 min", "Italian", "Main");

        // Test valid setter
        testRecipe.setTitle("New Valid Title");
        assert(testRecipe.getTitle() == "New Valid Title");

        // Test invalid setter
        try {
            testRecipe.setTitle("");
            return false; // Should have thrown exception
        } catch (const recipe::ValidationError&) {
            // Expected
        }
    } catch (const std::exception& e) {
        std::cerr << "Setter validation test failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// Test database operations (SQLite)
bool testDatabaseOperations() {
    try {
        // Use a test database file
        std::string testDbPath = "test_recipes.db";

        // Clean up any existing test database
        std::remove(testDbPath.c_str());

        RecipeManagerSQLite manager(testDbPath);

        // Test connection
        if (!manager.isConnected()) {
            std::cout << "Failed to connect to SQLite database" << std::endl;
            return false;
        }

        // Test adding a recipe
        recipe testRecipe("Test Recipe", "ingredient1, ingredient2", "step1, step2",
                         "2 servings", "15 minutes", "Test", "Test");

        if (!manager.addRecipe(testRecipe)) {
            std::cerr << "Failed to add test recipe" << std::endl;
            return false;
        }

        // Test viewing recipes
        auto recipes = manager.getAllRecipes();
        if (recipes.empty()) {
            std::cerr << "No recipes found after adding test recipe" << std::endl;
            return false;
        }

        // Test updating a recipe
        recipe updatedRecipe("Updated Recipe", "new ingredient", "new step",
                            "3 servings", "20 minutes", "Updated", "Updated");

        std::string recipeId = recipes[0].getId();
        if (!manager.updateRecipe(recipeId, updatedRecipe)) {
            std::cerr << "Failed to update test recipe" << std::endl;
            return false;
        }

        // Test getting a specific recipe
        auto retrievedRecipe = manager.getRecipe(recipeId);
        if (!retrievedRecipe || retrievedRecipe->getTitle() != "Updated Recipe") {
            std::cerr << "Failed to retrieve updated recipe" << std::endl;
            return false;
        }

        // Test search
        auto searchResults = manager.searchByTitle("Updated");
        if (searchResults.empty()) {
            std::cerr << "Search failed to find updated recipe" << std::endl;
            return false;
        }

        // Test deletion
        if (!manager.deleteRecipe(recipeId)) {
            std::cerr << "Failed to delete test recipe" << std::endl;
            return false;
        }

        // Verify deletion
        auto finalRecipes = manager.getAllRecipes();
        if (finalRecipes.size() != 0) {
            std::cerr << "Recipe still exists after deletion" << std::endl;
            return false;
        }

        // Clean up test database
        std::remove(testDbPath.c_str());

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Database test failed with exception: " << e.what() << std::endl;
        return false;
    }
}

// Test error handling scenarios
bool testErrorHandling() {
    // Test invalid database path
    try {
        RecipeManagerSQLite manager("/invalid/path/to/database.db");
        // Try to perform an operation that would require database access
        auto recipes = manager.getAllRecipes();
        std::cerr << "Expected exception for invalid database path" << std::endl;
        return false;
    } catch (const std::exception&) {
        // Expected - invalid path should cause exception
    }

    // Test operations on non-existent database
    try {
        RecipeManagerSQLite manager("/tmp/non_existent_test.db");
        // This should work - SQLite will create the database
        recipe testRecipe("Test", "Ingredients", "Instructions", "4", "30min", "Italian", "Main");
        auto result = manager.addRecipe(testRecipe);
        if (!result) {
            std::cerr << "Failed to add recipe to new database" << std::endl;
            return false;
        }
        // Clean up
        std::remove("/tmp/non_existent_test.db");
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception with new database: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// Test edge cases
bool testEdgeCases() {
    // Test maximum length inputs
    try {
        std::string maxTitle(100, 'A');
        std::string maxIngredients(1000, 'B');
        std::string maxInstructions(2000, 'C');
        std::string maxOther(50, 'D');

        recipe maxRecipe(maxTitle, maxIngredients, maxInstructions,
                        maxOther, maxOther, maxOther, maxOther);

        assert(maxRecipe.getTitle().length() == 100);
        assert(maxRecipe.getIngredients().length() == 1000);
        assert(maxRecipe.getInstructions().length() == 2000);
    } catch (const std::exception& e) {
        std::cerr << "Maximum length test failed: " << e.what() << std::endl;
        return false;
    }

    // Test special characters in inputs
    try {
        recipe specialRecipe("Pasta with spécial chärs", "Ingrédients: pâtes, œufs",
                           "Instructions with émojis 😀", "4 portions", "20 mins",
                           "Français", "Plat principal");

        assert(specialRecipe.getTitle() == "Pasta with spécial chärs");
    } catch (const std::exception& e) {
        std::cerr << "Special characters test failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// Main test runner
int main() {
    std::cout << "=== RecipeForADisaster Integration Tests ===\n" << std::endl;

    // Run all test suites
    runTest("Recipe Validation", testRecipeValidation);
    runTest("Database Operations", testDatabaseOperations);
    runTest("Error Handling", testErrorHandling);
    runTest("Edge Cases", testEdgeCases);

    // Print test summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Tests Run: " << testsRun << std::endl;
    std::cout << "Tests Passed: " << testsPassed << std::endl;
    std::cout << "Tests Failed: " << testsFailed << std::endl;

    if (testsFailed == 0) {
        std::cout << "\n🎉 All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Some tests failed. Please review the output above." << std::endl;
        return 1;
    }
}