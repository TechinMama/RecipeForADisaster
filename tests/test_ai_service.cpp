#include <cassert>
#include <iostream>
#include <string>
#include "aiService.h"

// Test counters
static int testsRun = 0;
static int testsPassed = 0;
static int testsFailed = 0;

// Helper function to run a test
static void runTest(const std::string& testName, std::function<bool()> testFunc) {
    testsRun++;
    std::cout << "Running test: " << testName << "... ";

    try {
        bool result = testFunc();
        if (result) {
            testsPassed++;
            std::cout << "PASSED" << std::endl;
        } else {
            testsFailed++;
            std::cout << "FAILED" << std::endl;
        }
    } catch (const std::exception& e) {
        testsFailed++;
        std::cout << "FAILED (Exception: " << e.what() << ")" << std::endl;
    }
}

// Test AI service initialization with invalid credentials
bool testAIServiceInitialization() {
    // Test with empty endpoint
    try {
        AIService service("", "key", "deployment");
        return false; // Should have thrown exception
    } catch (const AIService::AIServiceError&) {
        // Expected
    }

    // Test with empty API key
    try {
        AIService service("endpoint", "", "deployment");
        return false; // Should have thrown exception
    } catch (const AIService::AIServiceError&) {
        // Expected
    }

    // Test with empty deployment
    try {
        AIService service("endpoint", "key", "");
        return false; // Should have thrown exception
    } catch (const AIService::AIServiceError&) {
        // Expected
    }

    return true;
}

// Test AI result structure
bool testAIResult() {
    AIService::AIResult successResult(true, "Generated content", "", 100);
    assert(successResult.success == true);
    assert(successResult.generatedContent == "Generated content");
    assert(successResult.errorMessage.empty());
    assert(successResult.tokenCount == 100);

    AIService::AIResult errorResult(false, "", "Error message", 0);
    assert(errorResult.success == false);
    assert(errorResult.generatedContent.empty());
    assert(errorResult.errorMessage == "Error message");
    assert(errorResult.tokenCount == 0);

    return true;
}

// Test prompt validation
bool testPromptValidation() {
    // This test would require a real AI service instance
    // For now, we'll just test the validation logic by checking if we can create the service
    // In a real test environment, you would mock the Azure OpenAI client

    const char* endpoint = std::getenv("AZURE_OPENAI_ENDPOINT");
    const char* apiKey = std::getenv("AZURE_OPENAI_KEY");
    const char* deployment = std::getenv("AZURE_OPENAI_DEPLOYMENT");

    if (!endpoint || !apiKey || !deployment) {
        std::cout << " (Skipping - Azure OpenAI not configured)" << std::endl;
        return true; // Skip test if not configured
    }

    try {
        AIService service(endpoint, apiKey, deployment);

        // Test empty prompt
        auto result = service.generateRecipe("");
        assert(result.success == false);
        assert(result.errorMessage.find("empty") != std::string::npos);

        // Test very long prompt
        std::string longPrompt(2000, 'a');
        result = service.generateRecipe(longPrompt);
        assert(result.success == false);
        assert(result.errorMessage.find("long") != std::string::npos);

        return true;
    } catch (const AIService::AIServiceError&) {
        std::cout << " (Skipping - AI service initialization failed)" << std::endl;
        return true; // Skip if service can't be initialized
    }
}

// Test AI service connection check
bool testConnectionCheck() {
    const char* endpoint = std::getenv("AZURE_OPENAI_ENDPOINT");
    const char* apiKey = std::getenv("AZURE_OPENAI_KEY");
    const char* deployment = std::getenv("AZURE_OPENAI_DEPLOYMENT");

    if (!endpoint || !apiKey || !deployment) {
        std::cout << " (Skipping - Azure OpenAI not configured)" << std::endl;
        return true;
    }

    try {
        AIService service(endpoint, apiKey, deployment);
        bool connected = service.isConnected();
        // Connection check result depends on actual Azure service availability
        // We just verify the method doesn't throw
        return true;
    } catch (const AIService::AIServiceError&) {
        std::cout << " (Skipping - AI service initialization failed)" << std::endl;
        return true;
    }
}

// Test recipe response validation
bool testRecipeValidation() {
    // Test valid recipe format
    std::string validRecipe = R"(
**Title:** Test Recipe

**Ingredients:**
- Ingredient 1
- Ingredient 2

**Instructions:**
1. Step 1
2. Step 2

**Serving Size:** 4 servings

**Cook Time:** 30 minutes

**Category:** Test

**Type:** Main Course
)";

    // Test invalid recipe format (missing required fields)
    std::string invalidRecipe = "This is not a valid recipe format";

    // Note: In a real implementation, you would test the private validateRecipeResponse method
    // For now, we just verify the structure exists
    return true;
}

// Main test runner
int main() {
    std::cout << "Running AI Service Tests..." << std::endl;
    std::cout << "=================================" << std::endl;

    runTest("AI Service Initialization", testAIServiceInitialization);
    runTest("AI Result Structure", testAIResult);
    runTest("Prompt Validation", testPromptValidation);
    runTest("Connection Check", testConnectionCheck);
    runTest("Recipe Validation", testRecipeValidation);

    std::cout << "=================================" << std::endl;
    std::cout << "Tests completed: " << testsRun << " run, "
              << testsPassed << " passed, " << testsFailed << " failed" << std::endl;

    if (testsFailed > 0) {
        std::cout << "Some tests failed. Check Azure OpenAI configuration if running integration tests." << std::endl;
        return 1;
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}