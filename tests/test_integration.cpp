#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "recipe.h"
#include "recipeManager.h"

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

// Test database operations (requires MongoDB connection)
bool testDatabaseOperations() {
    try {
        // Initialize MongoDB instance
        mongocxx::instance instance{};

        // Use a test database URI - this should be set to a test database
        std::string testUri = std::getenv("MONGODB_TEST_URI") ?
                             std::getenv("MONGODB_TEST_URI") :
                             "mongodb://localhost:27017";

        recipeManager manager(testUri);

        // Test connection
        if (!manager.isConnected()) {
            std::cout << "Skipping database tests - MongoDB not available" << std::endl;
            return true; // Skip test if MongoDB not available
        }

        // Test adding a recipe
        recipe testRecipe("Test Recipe", "Test Ingredients", "Test Instructions",
                         "2 servings", "15 minutes", "Test", "Test");

        auto addResult = manager.addRecipe(testRecipe);
        if (!addResult.success) {
            std::cerr << "Failed to add test recipe: " << addResult.errorMessage << std::endl;
            return false;
        }

        // Test viewing recipes
        auto recipes = manager.viewRecipes();
        if (recipes.empty()) {
            std::cerr << "No recipes found after adding test recipe" << std::endl;
            return false;
        }

        // Test updating a recipe
        recipe updatedRecipe("Test Recipe", "Updated Ingredients", "Updated Instructions",
                           "3 servings", "20 minutes", "Updated", "Updated");

        auto updateResult = manager.updateRecipe("Test Recipe", updatedRecipe);
        if (!updateResult.success) {
            std::cerr << "Failed to update test recipe: " << updateResult.errorMessage << std::endl;
            return false;
        }

        // Test searching recipes
        auto searchResults = manager.searchRecipes("Test");
        if (searchResults.empty()) {
            std::cerr << "Search failed to find test recipe" << std::endl;
            return false;
        }

        // Test deleting a recipe
        auto deleteResult = manager.deleteRecipe("Test Recipe");
        if (!deleteResult.success) {
            std::cerr << "Failed to delete test recipe: " << deleteResult.errorMessage << std::endl;
            return false;
        }

        // Verify deletion
        auto finalRecipes = manager.viewRecipes();
        for (const auto& r : finalRecipes) {
            if (r.getTitle() == "Test Recipe") {
                std::cerr << "Test recipe still exists after deletion" << std::endl;
                return false;
            }
        }

    } catch (const recipeManager::DatabaseError& e) {
        std::cerr << "Database error during testing: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error during database testing: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// Test error handling scenarios
bool testErrorHandling() {
    // Test invalid MongoDB URI
    try {
        mongocxx::instance instance{};
        recipeManager manager("invalid://uri");
        return false; // Should have thrown exception
    } catch (const recipeManager::DatabaseError&) {
        // Expected
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception type: " << e.what() << std::endl;
        return false;
    }

    // Test empty MongoDB URI
    try {
        mongocxx::instance instance{};
        recipeManager manager("");
        return false; // Should have thrown exception
    } catch (const recipeManager::DatabaseError&) {
        // Expected
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception type: " << e.what() << std::endl;
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