#ifndef VAULT_CLIENT_HPP
#define VAULT_CLIENT_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class VaultClient {
public:
    VaultClient(const std::string& vault_addr, const std::string& token);
    ~VaultClient();

    // Initialize the client (setup curl, etc.)
    bool initialize();

    // Read a secret from Vault
    bool readSecret(const std::string& path, std::unordered_map<std::string, std::string>& secrets);

    // Write a secret to Vault
    bool writeSecret(const std::string& path, const std::unordered_map<std::string, std::string>& secrets);

    // Check if Vault is reachable and authenticated
    bool isHealthy();

    // Get last error message
    std::string getLastError() const { return last_error_; }

private:
    std::string vault_addr_;
    std::string token_;
    CURL* curl_;
    std::string last_error_;

    // Helper function to make HTTP requests
    bool makeRequest(const std::string& method, const std::string& url,
                    const std::string& data, std::string& response, long& http_code);

    // CURL write callback
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
};

#endif // VAULT_CLIENT_HPP