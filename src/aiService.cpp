#include "aiService.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "vaultService.h"  // Include Vault service header
#include "common_utils.h"  // Include common utilities

AIService::AIService(const std::string& endpoint, const std::string& apiKey, const std::string& deploymentName)
    : endpoint_(endpoint), apiKey_(apiKey), deploymentName_(deploymentName) {

    if (endpoint_.empty() || apiKey_.empty() || deploymentName_.empty()) {
        throw AIServiceError("Azure OpenAI configuration is incomplete. Please check endpoint, API key, and deployment name.");
    }

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

AIService::AIService(VaultService* vaultService, const std::string& vaultPath)
    : endpoint_(""), apiKey_(""), deploymentName_("") {

    try {
        if (!vaultService) {
            throw AIServiceError("Vault service is required for secure credential retrieval");
        }

        // Retrieve Azure OpenAI credentials from Vault
        auto endpointResult = vaultService->getSecret(vaultPath, "endpoint");
        auto apiKeyResult = vaultService->getSecret(vaultPath, "api_key");
        auto deploymentResult = vaultService->getSecret(vaultPath, "deployment_name");

        if (!endpointResult.success) {
            throw AIServiceError("Failed to retrieve Azure OpenAI endpoint from Vault: " + endpointResult.errorMessage);
        }
        if (!apiKeyResult.success) {
            throw AIServiceError("Failed to retrieve Azure OpenAI API key from Vault: " + apiKeyResult.errorMessage);
        }
        if (!deploymentResult.success) {
            throw AIServiceError("Failed to retrieve Azure OpenAI deployment name from Vault: " + deploymentResult.errorMessage);
        }

        endpoint_ = endpointResult.value;
        apiKey_ = apiKeyResult.value;
        deploymentName_ = deploymentResult.value;

        if (endpoint_.empty() || apiKey_.empty() || deploymentName_.empty()) {
            throw AIServiceError("Retrieved Azure OpenAI credentials from Vault are incomplete");
        }

        // Initialize libcurl
        curl_global_init(CURL_GLOBAL_DEFAULT);

        std::cout << "AI service initialized successfully using Vault credentials" << std::endl;
    } catch (const std::exception& e) {
        throw AIServiceError("Failed to initialize AI service with Vault: " + std::string(e.what()));
    }
}

AIService::~AIService() {
    curl_global_cleanup();
}

AIService::AIResult AIService::generateRecipe(const std::string& prompt) {
    if (prompt.empty()) {
        return AIResult(false, "", "Prompt cannot be empty", 0);
    }

    if (prompt.length() > 1000) {
        return AIResult(false, "", "Prompt is too long (maximum 1000 characters)", 0);
    }

    try {
        // Create JSON payload for Azure OpenAI API
        nlohmann::json requestJson = {
            {"messages", {
                {
                    {"role", "system"},
                    {"content", createSystemPrompt()}
                },
                {
                    {"role", "user"},
                    {"content", "Generate a recipe for: " + prompt}
                }
            }},
            {"max_tokens", 1000},
            {"temperature", 0.7},
            {"top_p", 0.95},
            {"frequency_penalty", 0},
            {"presence_penalty", 0}
        };

        std::string requestBody = requestJson.dump();

        // Initialize libcurl
        CURL* curl = curl_easy_init();
        if (!curl) {
            return AIResult(false, "", "Failed to initialize HTTP client", 0);
        }

        std::string response;
        std::string url = endpoint_ + "/openai/deployments/" + deploymentName_ + "/chat/completions?api-version=2023-12-01-preview";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("api-key: " + apiKey_).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestBody.length());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return AIResult(false, "", "HTTP request failed: " + std::string(curl_easy_strerror(res)), 0);
        }

        // Parse response
        auto responseJson = nlohmann::json::parse(response, nullptr, false);
        if (responseJson.is_discarded()) {
            return AIResult(false, "", "Failed to parse API response", 0);
        }

        if (responseJson.contains("error")) {
            std::string errorMsg = responseJson["error"]["message"];
            return AIResult(false, "", "Azure OpenAI API error: " + errorMsg, 0);
        }

        if (!responseJson.contains("choices") || responseJson["choices"].empty()) {
            return AIResult(false, "", "No response generated by AI", 0);
        }

        std::string aiResponse = responseJson["choices"][0]["message"]["content"];
        std::string parsedRecipe = parseRecipeResponse(aiResponse);

        if (!validateRecipeResponse(parsedRecipe)) {
            return AIResult(false, "", "Generated recipe format is invalid", responseJson.value("usage", nlohmann::json{}).value("total_tokens", 0));
        }

        int tokensUsed = responseJson.value("usage", nlohmann::json{}).value("total_tokens", 0);
        return AIResult(true, parsedRecipe, "", tokensUsed);

    } catch (const std::exception& e) {
        return AIResult(false, "", "Unexpected error: " + std::string(e.what()), 0);
    }
}

std::vector<AIService::AIResult> AIService::generateRecipeSuggestions(const std::string& prompt, int count) {
    std::vector<AIResult> results;

    if (count < 1 || count > 5) {
        results.push_back(AIResult(false, "", "Count must be between 1 and 5", 0));
        return results;
    }

    for (int i = 0; i < count; ++i) {
        std::string enhancedPrompt = prompt + " (variation " + std::to_string(i + 1) + ")";
        results.push_back(generateRecipe(enhancedPrompt));
    }

    return results;
}

bool AIService::isConnected() {
    try {
        // Simple test call to verify connection
        nlohmann::json testRequest = {
            {"messages", {{
                {"role", "user"},
                {"content", "Hello"}
            }}},
            {"max_tokens", 10}
        };

        std::string requestBody = testRequest.dump();

        CURL* curl = curl_easy_init();
        if (!curl) {
            return false;
        }

        std::string response;
        std::string url = endpoint_ + "/openai/deployments/" + deploymentName_ + "/chat/completions?api-version=2023-12-01-preview";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("api-key: " + apiKey_).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestBody.length());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return false;
        }

        auto responseJson = nlohmann::json::parse(response, nullptr, false);
        return !responseJson.is_discarded() && !responseJson.contains("error");

    } catch (const std::exception&) {
        return false;
    }
}

std::string AIService::createSystemPrompt() const {
    return R"(
You are a professional chef and recipe expert. Generate detailed, practical recipes based on user requests.

For each recipe, provide the following information in a structured format:

**Title:** [Recipe Name]

**Ingredients:**
- [Ingredient 1 with quantity]
- [Ingredient 2 with quantity]
- [Continue for all ingredients]

**Instructions:**
1. [Step 1]
2. [Step 2]
3. [Continue for all steps]

**Serving Size:** [Number of servings]

**Cook Time:** [Estimated time, e.g., "30 minutes"]

**Category:** [Cuisine type, e.g., "Italian", "Mexican", "American"]

**Type:** [Meal type, e.g., "Main Course", "Dessert", "Appetizer", "Breakfast"]

Ensure the recipe is:
- Practical and easy to follow
- Uses common ingredients when possible
- Includes accurate measurements
- Has clear, step-by-step instructions
- Appropriate serving sizes and cooking times
- Safe cooking practices

Keep the response focused on the recipe structure above.
)";
}

std::string AIService::parseRecipeResponse(const std::string& aiResponse) const {
    // For now, return the response as-is since the AI should format it correctly
    // In a production system, you might want to parse and validate the structure
    return aiResponse;
}

bool AIService::validateRecipeResponse(const std::string& response) const {
    // Basic validation - check if response contains required fields
    if (response.empty()) {
        return false;
    }

    // Check for minimum required content
    std::vector<std::string> requiredFields = {
        "**Title:**", "**Ingredients:**", "**Instructions:**",
        "**Serving Size:**", "**Cook Time:**", "**Category:**", "**Type:**"
    };

    for (const auto& field : requiredFields) {
        if (response.find(field) == std::string::npos) {
            return false;
        }
    }

    return true;
}