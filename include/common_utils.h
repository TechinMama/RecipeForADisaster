#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <string>
#include <curl/curl.h>

// Common callback function for libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);

#endif // COMMON_UTILS_H