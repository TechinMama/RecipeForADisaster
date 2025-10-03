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

# Add Crow web framework
include(FetchContent)
FetchContent_Declare(
  crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG v1.2.0
)
# Ensure Crow uses the same toolchain
set(CROW_CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
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

message(STATUS "VCPKG_ROOT: $ENV{VCPKG_ROOT}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# Try to find MongoDB C++ driver using CMake config (should work with vcpkg toolchain)
find_package(mongocxx CONFIG QUIET)
find_package(bsoncxx CONFIG QUIET)

# Check if imported targets are available
if(TARGET mongo::mongocxx_static AND TARGET mongo::bsoncxx_static)
    message(STATUS "Found MongoDB C++ driver via imported targets (static)")
    set(MONGODB_LIBRARIES mongo::mongocxx_static mongo::bsoncxx_static)
elseif(TARGET mongo::mongocxx_shared AND TARGET mongo::bsoncxx_shared)
    message(STATUS "Found MongoDB C++ driver via imported targets (shared)")
    set(MONGODB_LIBRARIES mongo::mongocxx_shared mongo::bsoncxx_shared)
elseif(TARGET mongocxx::mongocxx AND TARGET bsoncxx::bsoncxx)
    message(STATUS "Found MongoDB C++ driver via legacy imported targets")
    set(MONGODB_LIBRARIES mongocxx::mongocxx bsoncxx::bsoncxx)
else()
    message(STATUS "MongoDB imported targets not found, using manual paths for manifest mode")

    # In manifest mode, vcpkg installs to CMAKE_BINARY_DIR/vcpkg_installed
    set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")

    # Find MongoDB headers
    find_path(MONGOCXX_INCLUDE_DIR
        NAMES mongocxx/client.hpp
        PATHS
            ${VCPKG_INSTALLED_DIR}/include
            $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include
        PATH_SUFFIXES mongocxx/v_noabi
        NO_DEFAULT_PATH
    )

    find_path(BSONCXX_INCLUDE_DIR
        NAMES bsoncxx/json.hpp
        PATHS
            ${VCPKG_INSTALLED_DIR}/include
            $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include
        PATH_SUFFIXES bsoncxx/v_noabi
        NO_DEFAULT_PATH
    )

    # Find MongoDB libraries
    find_library(MONGOCXX_LIBRARY
        NAMES mongocxx-static mongocxx
        PATHS
            ${VCPKG_INSTALLED_DIR}/lib
            $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib
        NO_DEFAULT_PATH
    )

    find_library(BSONCXX_LIBRARY
        NAMES bsoncxx-static bsoncxx
        PATHS
            ${VCPKG_INSTALLED_DIR}/lib
            $ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib
        NO_DEFAULT_PATH
    )

    if(MONGOCXX_INCLUDE_DIR AND BSONCXX_INCLUDE_DIR AND MONGOCXX_LIBRARY AND BSONCXX_LIBRARY)
        message(STATUS "Found MongoDB manually: ${MONGOCXX_INCLUDE_DIR}, ${MONGOCXX_LIBRARY}")
        include_directories(${MONGOCXX_INCLUDE_DIR}/../.. ${BSONCXX_INCLUDE_DIR}/../..)
        set(MONGODB_LIBRARIES ${MONGOCXX_LIBRARY} ${BSONCXX_LIBRARY})
    else()
        message(FATAL_ERROR "Could not find MongoDB C++ driver. MONGOCXX_INCLUDE_DIR: ${MONGOCXX_INCLUDE_DIR}, BSONCXX_INCLUDE_DIR: ${BSONCXX_INCLUDE_DIR}, MONGOCXX_LIBRARY: ${MONGOCXX_LIBRARY}, BSONCXX_LIBRARY: ${BSONCXX_LIBRARY}")
    endif()
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
target_link_libraries(RecipeForADisaster ${MONGODB_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(web_server Crow::Crow ${MONGODB_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(integration_tests ${MONGODB_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
target_link_libraries(ai_service_tests ${MONGODB_LIBRARIES} CURL::libcurl nlohmann_json::nlohmann_json)
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
