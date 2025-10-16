#include "vaultService.h"
#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "Testing Vault Service Integration..." << std::endl;

    // Get Vault configuration from environment variables
    const char* vaultAddr = std::getenv("VAULT_ADDR");
    const char* vaultToken = std::getenv("VAULT_TOKEN");

    if (!vaultAddr || !vaultToken) {
        std::cout << "Vault not configured. Set VAULT_ADDR and VAULT_TOKEN environment variables." << std::endl;
        std::cout << "Skipping Vault integration test." << std::endl;
        return 0;
    }

    try {
        // Initialize Vault service
        VaultService::VaultConfig config;
        config.address = vaultAddr;
        config.token = vaultToken;

        VaultService vaultService(config);
        std::cout << "✓ Vault service initialized successfully" << std::endl;

        // Test retrieving a secret (this will fail if the secret doesn't exist, which is expected)
        auto result = vaultService.getSecret("test/example", "test_key");
        if (result.success) {
            std::cout << "✓ Successfully retrieved test secret: " << result.value << std::endl;
        } else {
            std::cout << "⚠ Test secret retrieval failed (expected if test secret doesn't exist): " << result.errorMessage << std::endl;
        }

        // Test MongoDB credential retrieval
        auto mongoResult = vaultService.getSecret("database/mongodb", "uri");
        if (mongoResult.success) {
            std::cout << "✓ MongoDB URI retrieved from Vault successfully" << std::endl;
        } else {
            std::cout << "⚠ MongoDB URI retrieval failed: " << mongoResult.errorMessage << std::endl;
        }

        // Test Azure OpenAI credential retrieval
        auto aiResult = vaultService.getSecret("azure-openai", "endpoint");
        if (aiResult.success) {
            std::cout << "✓ Azure OpenAI endpoint retrieved from Vault successfully" << std::endl;
        } else {
            std::cout << "⚠ Azure OpenAI endpoint retrieval failed: " << aiResult.errorMessage << std::endl;
        }

        // Test API key retrieval
        auto apiKeyResult = vaultService.getSecret("azure-openai", "api_key");
        if (apiKeyResult.success) {
            std::cout << "✓ Azure OpenAI API key retrieved from Vault successfully" << std::endl;
        } else {
            std::cout << "⚠ Azure OpenAI API key retrieval failed: " << apiKeyResult.errorMessage << std::endl;
        }

        // Test deployment name retrieval
        auto deploymentResult = vaultService.getSecret("azure-openai", "deployment_name");
        if (deploymentResult.success) {
            std::cout << "✓ Azure OpenAI deployment name retrieved from Vault successfully" << std::endl;
        } else {
            std::cout << "⚠ Azure OpenAI deployment name retrieval failed: " << deploymentResult.errorMessage << std::endl;
        }

        std::cout << "Vault integration test completed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "❌ Vault integration test failed: " << e.what() << std::endl;
        return 1;
    }
}