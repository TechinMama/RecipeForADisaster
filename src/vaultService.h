#ifndef VAULT_SERVICE_H
#define VAULT_SERVICE_H

#include <string>
#include <memory>
#include "vault_client.hpp"

class VaultService {
public:
    struct VaultConfig {
        std::string address;
        std::string token;
        std::string mountPath = "secret";  // Default KV mount path
    };

    struct CredentialResult {
        bool success;
        std::string value;
        std::string errorMessage;
    };

    VaultService(const VaultConfig& config);
    ~VaultService();

    // Retrieve a secret from Vault
    CredentialResult getSecret(const std::string& path, const std::string& key);

    // Check if Vault connection is healthy
    bool isConnected() const;

private:
    VaultConfig config_;
    std::unique_ptr<VaultClient> vault_client_;
    bool connected_;
};

#endif // VAULT_SERVICE_H