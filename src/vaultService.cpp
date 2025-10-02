#include "vaultService.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "common_utils.h"  // Include common utilities

VaultService::VaultService(const VaultConfig& config) : config_(config), curl_(nullptr), connected_(false) {
    try {
        if (config.address.empty() || config.token.empty()) {
            throw std::runtime_error("Vault address and token are required");
        }

        // Initialize libcurl
        curl_ = curl_easy_init();
        if (!curl_) {
            throw std::runtime_error("Failed to initialize libcurl");
        }

        // Test connection by making a simple request
        connected_ = isConnected();

        if (!connected_) {
            throw std::runtime_error("Failed to connect to Vault server");
        }

        std::cout << "Vault service initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Vault service: " << e.what() << std::endl;
        connected_ = false;
        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
        throw;
    }
}

VaultService::~VaultService() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

VaultService::CredentialResult VaultService::getSecret(const std::string& path, const std::string& key) {
    try {
        if (!connected_ || !curl_) {
            return CredentialResult{false, "", "Vault service is not connected"};
        }

        // Make request to Vault KV API
        std::string endpoint = "/v1/" + config_.mountPath + "/data/" + path;
        std::string response = makeVaultRequest(endpoint);

        if (response.empty()) {
            return CredentialResult{false, "", "Empty response from Vault"};
        }

        // Parse JSON response
        auto responseJson = nlohmann::json::parse(response, nullptr, false);
        if (responseJson.is_discarded()) {
            return CredentialResult{false, "", "Failed to parse Vault response"};
        }

        // Check for errors
        if (responseJson.contains("errors")) {
            std::string errorMsg = "Vault API error";
            if (responseJson["errors"].is_array() && !responseJson["errors"].empty()) {
                errorMsg += ": " + responseJson["errors"][0].get<std::string>();
            }
            return CredentialResult{false, "", errorMsg};
        }

        // Extract the secret data
        if (responseJson.contains("data") && responseJson["data"].contains("data")) {
            auto data = responseJson["data"]["data"];
            if (data.contains(key)) {
                return CredentialResult{true, data[key].get<std::string>(), ""};
            } else {
                return CredentialResult{false, "", "Key '" + key + "' not found in secret"};
            }
        } else {
            return CredentialResult{false, "", "Invalid Vault response format"};
        }

    } catch (const std::exception& e) {
        return CredentialResult{false, "", "Error retrieving secret: " + std::string(e.what())};
    }
}

bool VaultService::isConnected() const {
    try {
        if (!curl_) {
            return false;
        }

        // Try to access Vault health endpoint or make a simple authenticated request
        // For now, we'll assume connection is valid if curl is initialized
        // In a production environment, you might want to implement a health check
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string VaultService::makeVaultRequest(const std::string& endpoint) {
    if (!curl_) {
        return "";
    }

    std::string response;
    std::string url = config_.address + endpoint;

    // Set up headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-Vault-Token: " + config_.token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configure curl
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);  // Verify SSL certificates
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);

    // Perform request
    CURLcode res = curl_easy_perform(curl_);

    // Clean up headers
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "Vault HTTP request failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}