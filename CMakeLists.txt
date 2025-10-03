cmake_minimum_required(VERSION 3.10)

# Add common installation paths to CMAKE_PREFIX_PATH before project()
list(APPEND CMAKE_PREFIX_PATH "/usr/local" "/opt/homebrew" "/usr" "/usr/local/lib/cmake" "/usr/lib/cmake")

# Add vcpkg paths if VCPKG_ROOT is set
if(DEFINED ENV{VCPKG_ROOT})
    if(WIN32)
        list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows")
        set(Boost_ROOT "$ENV{VCPKG_ROOT}/installed/x64-windows")
        list(APPEND CMAKE_IGNORE_PATH "/opt/anaconda3")
    elseif(APPLE)
        list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-osx" "$ENV{VCPKG_ROOT}/installed/arm64-osx")
        set(Boost_ROOT "$ENV{VCPKG_ROOT}/installed/x64-osx")
        list(APPEND CMAKE_IGNORE_PATH "/opt/anaconda3" "/opt/homebrew")
    else()
        list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-linux")
        set(Boost_ROOT "$ENV{VCPKG_ROOT}/installed/x64-linux")
        list(APPEND CMAKE_IGNORE_PATH "/opt/anaconda3")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17) # Set the C++ standard to C++17

project(RecipeForADisaster)

# Windows-specific compiler flags to suppress warnings
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(include src) # Add the 'include' and 'src' directories to the include path

# Enable testing
enable_testing()

# Add web framework dependencies
include(FetchContent)

# Add Crow web framework (commented out since not used)
# FetchContent_Declare(
#   crow
#   GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
#   GIT_TAG v1.2.0
# )
# # Ensure Crow uses the same toolchain
# set(CROW_CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
# FetchContent_MakeAvailable(crow)

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

# Find SQLite3 (much simpler than MongoDB!)
# CMAKE_PREFIX_PATH is set at the top of the file

message(STATUS "VCPKG_ROOT: $ENV{VCPKG_ROOT}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# Find SQLite3 (much simpler than MongoDB!)
find_package(SQLite3 REQUIRED)

if(SQLite3_FOUND)
    message(STATUS "Found SQLite3: ${SQLite3_INCLUDE_DIRS}")
    include_directories(${SQLite3_INCLUDE_DIRS})
    set(DATABASE_LIBRARIES SQLite::SQLite3)
else()
    message(FATAL_ERROR "SQLite3 not found")
endif()

# Main application sources (SQLite version)
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")  # Remove MongoDB main
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/recipeManager.cpp")  # Remove MongoDB manager

# Create main executable (SQLite version)
add_executable(RecipeForADisaster src/main_sqlite.cpp src/recipeManagerSQLite.cpp src/recipe.cpp src/aiService.cpp src/vaultService.cpp src/common_utils.cpp)

# Create test executables
add_executable(integration_tests tests/test_integration.cpp src/recipe.cpp src/recipeManagerSQLite.cpp src/vaultService.cpp src/common_utils.cpp)
add_executable(ai_service_tests tests/test_ai_service.cpp src/recipe.cpp src/recipeManagerSQLite.cpp src/aiService.cpp src/vaultService.cpp src/common_utils.cpp)
add_executable(vault_tests tests/test_vault.cpp src/vaultService.cpp src/common_utils.cpp)

# Link libraries for test executables
# Link libraries
target_link_libraries(RecipeForADisaster ${DATABASE_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(integration_tests ${DATABASE_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(ai_service_tests ${DATABASE_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(vault_tests CURL::libcurl nlohmann_json::nlohmann_json)

# Test executable needs the same include directories
target_include_directories(integration_tests PRIVATE include)
target_include_directories(ai_service_tests PRIVATE include)
target_include_directories(ai_service_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)
target_include_directories(vault_tests PRIVATE include)
target_include_directories(vault_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)
target_include_directories(RecipeForADisaster PRIVATE include)
target_include_directories(RecipeForADisaster PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)

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
