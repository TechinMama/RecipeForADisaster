cmake_minimum_required(VERSION 3.10)

# Add common installation paths to CMAKE_PREFIX_PATH before project()
list(APPEND CMAKE_PREFIX_PATH "/usr/local" "/opt/homebrew" "/usr" "/usr/local/lib/cmake" "/usr/lib/cmake")

set(CMAKE_CXX_STANDARD 17) # Set the C++ standard to C++17

project(RecipeForADisaster)

include_directories(include src) # Add the 'include' and 'src' directories to the include path

# Enable testing
enable_testing()

# Add Crow web framework
include(FetchContent)
FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

# Add Azure SDK for C++
FetchContent_Declare(
  azure-sdk-cpp
  GIT_REPOSITORY https://github.com/Azure/azure-sdk-for-cpp.git
  GIT_TAG azure-core_1.10.3
  SOURCE_SUBDIR sdk/core/azure-core
)
FetchContent_MakeAvailable(azure-sdk-cpp)

# Find libcurl for HTTP requests (already available from Azure SDK)
find_package(CURL REQUIRED)

# Add nlohmann/json for JSON parsing
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Add HashiCorp Vault C++ client
# Using a simple HTTP-based implementation instead of external library

# Remove Azure OpenAI SDK dependencies for now
# Add Azure SDK for C++ (keeping core for potential future use)
# Azure SDK is now handled via vcpkg on Windows, FetchContent on other platforms

# Find MongoDB C++ driver
# CMAKE_PREFIX_PATH is set at the top of the file

# Try to find using CMake config first (CI/CD and standard installations)
find_package(mongocxx QUIET)
find_package(bsoncxx QUIET)

# If not found, use manual paths
if(NOT mongocxx_FOUND OR NOT bsoncxx_FOUND)
    message(STATUS "MongoDB C++ driver not found via find_package, using manual paths")
    
    # Try to find headers automatically
    find_path(MONGOCXX_INCLUDE_DIR mongocxx/client.hpp
        PATHS /usr/local/include /opt/homebrew/include /usr/include
        PATH_SUFFIXES mongocxx/v_noabi mongocxx)
    
    find_path(BSONCXX_INCLUDE_DIR bsoncxx/document.hpp
        PATHS /usr/local/include /opt/homebrew/include /usr/include
        PATH_SUFFIXES bsoncxx/v_noabi bsoncxx)
    
    find_library(MONGOCXX_LIBRARY mongocxx
        PATHS /usr/local/lib /opt/homebrew/lib /usr/lib)
    
    find_library(BSONCXX_LIBRARY bsoncxx
        PATHS /usr/local/lib /opt/homebrew/lib /usr/lib)
    
    if(MONGOCXX_INCLUDE_DIR AND BSONCXX_INCLUDE_DIR)
        message(STATUS "Found MongoDB headers: ${MONGOCXX_INCLUDE_DIR}, ${BSONCXX_INCLUDE_DIR}")
        include_directories(${MONGOCXX_INCLUDE_DIR})
        include_directories(${BSONCXX_INCLUDE_DIR})
        
        if(MONGOCXX_LIBRARY AND BSONCXX_LIBRARY)
            message(STATUS "Found MongoDB libraries: ${MONGOCXX_LIBRARY}, ${BSONCXX_LIBRARY}")
        else()
            message(WARNING "MongoDB libraries not found, linking may fail")
        endif()
    else()
        message(FATAL_ERROR "Could not find MongoDB C++ driver headers. Searched in: /usr/local/include, /opt/homebrew/include, /usr/include")
    endif()
else()
    message(STATUS "Found MongoDB C++ driver via find_package")
endif()

# Main application sources (exclude web_server.cpp)
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/web_server.cpp")

# Create main executable
add_executable(RecipeForADisaster ${SOURCES})

# Create web server executable
add_executable(web_server src/web_server.cpp src/recipe.cpp src/recipeManager.cpp src/aiService.cpp src/vaultService.cpp src/common_utils.cpp)

# Create test executables
add_executable(integration_tests tests/test_integration.cpp src/recipe.cpp src/recipeManager.cpp src/vaultService.cpp src/common_utils.cpp)
add_executable(ai_service_tests tests/test_ai_service.cpp src/recipe.cpp src/recipeManager.cpp src/aiService.cpp src/vaultService.cpp src/common_utils.cpp)
add_executable(vault_tests tests/test_vault.cpp src/vaultService.cpp src/common_utils.cpp)

# Link libraries for test executables
# Link libraries
target_link_libraries(RecipeForADisaster mongocxx bsoncxx CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(web_server Crow::Crow mongocxx bsoncxx CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(integration_tests mongocxx bsoncxx CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(ai_service_tests mongocxx bsoncxx CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(vault_tests CURL::libcurl nlohmann_json::nlohmann_json)

# Test executable needs the same include directories
target_include_directories(integration_tests PRIVATE include)
target_include_directories(integration_tests PRIVATE /opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/include)
target_include_directories(ai_service_tests PRIVATE include)
target_include_directories(ai_service_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)
target_include_directories(ai_service_tests PRIVATE /opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/include)
target_include_directories(vault_tests PRIVATE include)
target_include_directories(vault_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)
target_include_directories(web_server PRIVATE include)
target_include_directories(web_server PRIVATE /opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/include)
target_include_directories(RecipeForADisaster PRIVATE include)
target_include_directories(RecipeForADisaster PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)
target_include_directories(RecipeForADisaster PRIVATE /opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/include)

# Add tests
add_test(NAME IntegrationTests COMMAND integration_tests)
add_test(NAME AIServiceTests COMMAND ai_service_tests)
add_test(NAME VaultTests COMMAND vault_tests)

# Optional: Add a custom target to run tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tests
    COMMENT "Running integration tests"
)
