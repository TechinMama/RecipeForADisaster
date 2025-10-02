cmake_minimum_required(VERSION 3.10)

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
FetchContent_Declare(
  azure-sdk-cpp
  GIT_REPOSITORY https://github.com/Azure/azure-sdk-for-cpp.git
  GIT_TAG azure-core_1.10.3
)
FetchContent_MakeAvailable(azure-sdk-cpp)

# Find MongoDB C++ driver
# set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/opt/homebrew/Cellar/mongo-cxx-driver/4.1.2")
# find_package(mongocxx REQUIRED)

# Manually set MongoDB paths
include_directories(/opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/include/mongocxx/v_noabi)
include_directories(/opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/include/bsoncxx/v_noabi)
link_directories(/opt/homebrew/Cellar/mongo-cxx-driver/4.1.2/lib)
find_package(bsoncxx REQUIRED)

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
