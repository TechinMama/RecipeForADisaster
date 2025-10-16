#include "vaultService.h"
#include <iostream>
#include <sstream>
#include "vault_client.hpp"
#include "common_utils.h"  // Include common utilities

VaultService::VaultService(const VaultConfig& config) : config_(config), connected_(false) {
    try {
        if (config.address.empty() || config.token.empty()) {
            throw std::runtime_error("Vault address and token are required");
        }

        // Initialize Vault client
        vault_client_ = std::make_unique<VaultClient>(config.address, config.token);
        if (!vault_client_->initialize()) {
            throw std::runtime_error("Failed to initialize Vault client: " + vault_client_->getLastError());
        }

        // Test connection
        connected_ = vault_client_->isHealthy();

        if (!connected_) {
            throw std::runtime_error("Failed to connect to Vault server");
        }

        std::cout << "Vault service initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Vault service: " << e.what() << std::endl;
        connected_ = false;
        vault_client_.reset();
        throw;
    }
}

VaultService::~VaultService() {
    // VaultClient destructor handles cleanup
}

VaultService::CredentialResult VaultService::getSecret(const std::string& path, const std::string& key) {
    try {
        if (!connected_ || !vault_client_) {
            return CredentialResult{false, "", "Vault service is not connected"};
        }

        // Construct full path with mount path
        std::string fullPath = config_.mountPath + "/data/" + path;

        std::unordered_map<std::string, std::string> secrets;
        if (!vault_client_->readSecret(fullPath, secrets)) {
            return CredentialResult{false, "", "Failed to read secret: " + vault_client_->getLastError()};
        }

        // Find the requested key
        auto it = secrets.find(key);
        if (it != secrets.end()) {
            return CredentialResult{true, it->second, ""};
        } else {
            return CredentialResult{false, "", "Key '" + key + "' not found in secret"};
        }

    } catch (const std::exception& e) {
        return CredentialResult{false, "", "Error retrieving secret: " + std::string(e.what())};
    }
}

bool VaultService::isConnected() const {
    return connected_ && vault_client_ && vault_client_->isHealthy();
}