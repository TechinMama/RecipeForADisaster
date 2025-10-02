#ifndef AI_SERVICE_H
#define AI_SERVICE_H

#include <string>
#include <vector>
#include <stdexcept>

class VaultService;  // Forward declaration

class AIService {
public:
    // Custom exception for AI service errors
    class AIServiceError : public std::runtime_error {
    public:
        explicit AIServiceError(const std::string& message) : std::runtime_error(message) {}
    };

    // Result structure for AI operations
    struct AIResult {
        bool success;
        std::string generatedContent;
        std::string errorMessage;
        int tokenCount;

        AIResult() : success(false), tokenCount(0) {}
        AIResult(bool s, const std::string& content, const std::string& error = "", int tokens = 0)
            : success(s), generatedContent(content), errorMessage(error), tokenCount(tokens) {}
    };

    // Constructor with direct credentials (for backward compatibility)
    AIService(const std::string& endpoint, const std::string& apiKey, const std::string& deploymentName);

    // Constructor with Vault service for secure credential retrieval
    AIService(VaultService* vaultService, const std::string& vaultPath = "ai/azure-openai");

    ~AIService();

    // Generate a recipe based on user prompt
    AIResult generateRecipe(const std::string& prompt);

    // Generate multiple recipe suggestions
    std::vector<AIResult> generateRecipeSuggestions(const std::string& prompt, int count = 3);

    // Validate connection to Azure OpenAI
    bool isConnected();

private:
    std::string endpoint_;
    std::string apiKey_;
    std::string deploymentName_;

    // Create the system prompt for recipe generation
    std::string createSystemPrompt() const;

    // Parse AI response and extract recipe content
    std::string parseRecipeResponse(const std::string& aiResponse) const;

    // Validate AI response format
    bool validateRecipeResponse(const std::string& response) const;
};

#endif // AI_SERVICE_H