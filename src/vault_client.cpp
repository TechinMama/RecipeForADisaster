#include "vault_client.hpp"
#include <iostream>
#include <sstream>

VaultClient::VaultClient(const std::string& vault_addr, const std::string& token)
    : vault_addr_(vault_addr), token_(token), curl_(nullptr) {
}

VaultClient::~VaultClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

bool VaultClient::initialize() {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        last_error_ = "Failed to initialize curl";
        return false;
    }

    curl_ = curl_easy_init();
    if (!curl_) {
        last_error_ = "Failed to create curl handle";
        return false;
    }

    // Set common curl options
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L); // For development only
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L); // For development only
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);

    return true;
}

bool VaultClient::readSecret(const std::string& path,
                           std::unordered_map<std::string, std::string>& secrets) {
    std::string url = vault_addr_ + "/v1/" + path;
    std::string response;
    long http_code;

    if (!makeRequest("GET", url, "", response, http_code)) {
        return false;
    }

    if (http_code != 200) {
        last_error_ = "HTTP " + std::to_string(http_code) + ": " + response;
        return false;
    }

    try {
        auto json_response = nlohmann::json::parse(response);
        if (json_response.contains("data") && json_response["data"].contains("data")) {
            auto data = json_response["data"]["data"];
            for (auto& [key, value] : data.items()) {
                secrets[key] = value.get<std::string>();
            }
            return true;
        } else {
            last_error_ = "Invalid response format";
            return false;
        }
    } catch (const std::exception& e) {
        last_error_ = "JSON parsing error: " + std::string(e.what());
        return false;
    }
}

bool VaultClient::writeSecret(const std::string& path,
                            const std::unordered_map<std::string, std::string>& secrets) {
    std::string url = vault_addr_ + "/v1/" + path;

    nlohmann::json json_data;
    json_data["data"] = secrets;

    std::string data = json_data.dump();
    std::string response;
    long http_code;

    if (!makeRequest("POST", url, data, response, http_code)) {
        return false;
    }

    if (http_code != 200 && http_code != 204) {
        last_error_ = "HTTP " + std::to_string(http_code) + ": " + response;
        return false;
    }

    return true;
}

bool VaultClient::isHealthy() {
    std::string url = vault_addr_ + "/v1/sys/health";
    std::string response;
    long http_code;

    if (!makeRequest("GET", url, "", response, http_code)) {
        return false;
    }

    return (http_code == 200 || http_code == 429 || http_code == 472 || http_code == 473);
}

bool VaultClient::makeRequest(const std::string& method, const std::string& url,
                            const std::string& data, std::string& response, long& http_code) {
    if (!curl_) {
        last_error_ = "Curl not initialized";
        return false;
    }

    response.clear();

    // Reset curl handle
    curl_easy_reset(curl_);

    // Set URL
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-Vault-Token: " + token_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

    // Set method
    if (method == "POST") {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        if (!data.empty()) {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
        }
    } else if (method == "PUT") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
        if (!data.empty()) {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
        }
    } else if (method == "DELETE") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    // GET is default

    // Set write callback
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    // Perform request
    CURLcode res = curl_easy_perform(curl_);

    // Get HTTP response code
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);

    // Clean up headers
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        last_error_ = "Curl error: " + std::string(curl_easy_strerror(res));
        return false;
    }

    return true;
}

size_t VaultClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}