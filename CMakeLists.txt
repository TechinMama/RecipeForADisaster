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

# Try to find using CMake config first (CI/CD and standard installations)
find_package(mongocxx QUIET)
find_package(bsoncxx QUIET)

# Check if imported targets are available
if(TARGET mongocxx::mongocxx AND TARGET bsoncxx::bsoncxx)
    message(STATUS "Found MongoDB C++ driver via imported targets")
    set(MONGODB_LIBRARIES mongocxx::mongocxx bsoncxx::bsoncxx)
    # Also add include directories if they are provided by the package
    if(MONGOCXX_INCLUDE_DIRS)
        include_directories(${MONGOCXX_INCLUDE_DIRS})
    endif()
    if(BSONCXX_INCLUDE_DIRS)
        include_directories(${BSONCXX_INCLUDE_DIRS})
    endif()
else()
    message(STATUS "MongoDB imported targets not found, using manual paths")
    
    # For CI environments or container builds, use known vcpkg installation paths
    if(DEFINED ENV{CI} OR DEFINED ENV{GITHUB_ACTIONS} OR "$ENV{VCPKG_ROOT}" STREQUAL "/vcpkg")
        message(STATUS "Using hardcoded vcpkg paths for CI/container environment")
        set(MONGODB_LIBRARIES 
            $ENV{VCPKG_ROOT}/installed/x64-linux/lib/libmongocxx.a
            $ENV{VCPKG_ROOT}/installed/x64-linux/lib/libbsoncxx.a
        )
        include_directories($ENV{VCPKG_ROOT}/installed/x64-linux/include)
    else()
        # Manual finding for local development
        find_path(MONGOCXX_INCLUDE_DIR mongocxx/client.hpp
            PATHS /usr/local/include /opt/homebrew/include /usr/include
                  $ENV{VCPKG_ROOT}/installed/x64-linux/include
                  $ENV{VCPKG_ROOT}/installed/x64-osx/include
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/include
                  $ENV{VCPKG_ROOT}/installed/x64-windows/include
            PATH_SUFFIXES mongocxx/v_noabi mongocxx "")
        
        message(STATUS "MONGOCXX_INCLUDE_DIR: ${MONGOCXX_INCLUDE_DIR}")
        
        # If not found, try the fallback paths
        if(NOT MONGOCXX_INCLUDE_DIR)
            find_path(MONGOCXX_INCLUDE_DIR mongocxx/client.hpp
                PATHS /usr/local/include/mongocxx/v_noabi /opt/homebrew/include/mongocxx/v_noabi /usr/include/mongocxx/v_noabi)
            if(MONGOCXX_INCLUDE_DIR)
                # Found in v_noabi, need to add parent directory
                get_filename_component(MONGOCXX_INCLUDE_DIR "${MONGOCXX_INCLUDE_DIR}" DIRECTORY)
            endif()
        endif()
        
        find_path(BSONCXX_INCLUDE_DIR bsoncxx/json.hpp
            PATHS /usr/local/include /opt/homebrew/include /usr/include
                  $ENV{VCPKG_ROOT}/installed/x64-linux/include
                  $ENV{VCPKG_ROOT}/installed/x64-osx/include
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/include
                  $ENV{VCPKG_ROOT}/installed/x64-windows/include
            PATH_SUFFIXES bsoncxx/v_noabi bsoncxx "" bsoncxx/v_noabi/bsoncxx)
        
        message(STATUS "BSONCXX_INCLUDE_DIR: ${BSONCXX_INCLUDE_DIR}")
        
        if(NOT BSONCXX_INCLUDE_DIR)
            find_path(BSONCXX_INCLUDE_DIR bsoncxx/json.hpp
                PATHS /usr/local/include/bsoncxx/v_noabi /opt/homebrew/include/bsoncxx/v_noabi /usr/include/bsoncxx/v_noabi
                      $ENV{VCPKG_ROOT}/installed/x64-linux/include/bsoncxx/v_noabi
                      $ENV{VCPKG_ROOT}/installed/arm64-osx/include/bsoncxx/v_noabi)
            if(BSONCXX_INCLUDE_DIR)
                get_filename_component(BSONCXX_INCLUDE_DIR "${BSONCXX_INCLUDE_DIR}" DIRECTORY)
            endif()
            message(STATUS "Fallback BSONCXX_INCLUDE_DIR: ${BSONCXX_INCLUDE_DIR}")
        endif()
        
        find_library(MONGOCXX_LIBRARY mongocxx-static
            PATHS /usr/local/lib /opt/homebrew/lib /usr/lib
                  $ENV{VCPKG_ROOT}/installed/x64-linux/lib
                  $ENV{VCPKG_ROOT}/installed/x64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/bin)
        
        message(STATUS "MONGOCXX_LIBRARY: ${MONGOCXX_LIBRARY}")
        
        find_library(BSONCXX_LIBRARY bsoncxx-static
            PATHS /usr/local/lib /opt/homebrew/lib /usr/lib
                  $ENV{VCPKG_ROOT}/installed/x64-linux/lib
                  $ENV{VCPKG_ROOT}/installed/x64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/bin)
        
        message(STATUS "BSONCXX_LIBRARY: ${BSONCXX_LIBRARY}")
        
        # Also find the underlying C libraries
        find_library(MONGOC_LIBRARY mongoc-static-1.0
            PATHS /usr/local/lib /opt/homebrew/lib /usr/lib
                  $ENV{VCPKG_ROOT}/installed/x64-linux/lib
                  $ENV{VCPKG_ROOT}/installed/x64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/bin)
        
        message(STATUS "MONGOC_LIBRARY: ${MONGOC_LIBRARY}")
        
        find_library(BSONC_LIBRARY bson-static-1.0
            PATHS /usr/local/lib /opt/homebrew/lib /usr/lib
                  $ENV{VCPKG_ROOT}/installed/x64-linux/lib
                  $ENV{VCPKG_ROOT}/installed/x64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/bin)
        
        message(STATUS "BSONC_LIBRARY: ${BSONC_LIBRARY}")
        
        # Find utf8proc library
        find_library(UTF8PROC_LIBRARY utf8proc
            PATHS /usr/local/lib /opt/homebrew/lib /usr/lib
                  $ENV{VCPKG_ROOT}/installed/x64-linux/lib
                  $ENV{VCPKG_ROOT}/installed/x64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/arm64-osx/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/lib
                  $ENV{VCPKG_ROOT}/installed/x64-windows/bin)
        
        message(STATUS "UTF8PROC_LIBRARY: ${UTF8PROC_LIBRARY}")
        
        # Try alternative library names
        if(NOT MONGOCXX_LIBRARY)
            find_library(MONGOCXX_LIBRARY libmongocxx
                PATHS $ENV{VCPKG_ROOT}/installed/x64-linux/lib)
            message(STATUS "Alternative MONGOCXX_LIBRARY: ${MONGOCXX_LIBRARY}")
        endif()
        
        if(NOT BSONCXX_LIBRARY)
            find_library(BSONCXX_LIBRARY libbsoncxx
                PATHS $ENV{VCPKG_ROOT}/installed/x64-linux/lib)
            message(STATUS "Alternative BSONCXX_LIBRARY: ${BSONCXX_LIBRARY}")
        endif()
        
        if(MONGOCXX_INCLUDE_DIR AND BSONCXX_INCLUDE_DIR)
            message(STATUS "Found MongoDB headers: ${MONGOCXX_INCLUDE_DIR}, ${BSONCXX_INCLUDE_DIR}")
            # For v_noabi includes, we need the base include directory and the v_noabi directory
            get_filename_component(MONGOCXX_TEMP_DIR "${MONGOCXX_INCLUDE_DIR}" DIRECTORY)  # remove v_noabi
            get_filename_component(MONGOCXX_BASE_DIR "${MONGOCXX_TEMP_DIR}" DIRECTORY)     # remove mongocxx
            get_filename_component(BSONCXX_TEMP_DIR "${BSONCXX_INCLUDE_DIR}" DIRECTORY)   # remove v_noabi  
            get_filename_component(BSONCXX_BASE_DIR "${BSONCXX_TEMP_DIR}" DIRECTORY)      # remove bsoncxx
            include_directories(${MONGOCXX_BASE_DIR})
            include_directories(${BSONCXX_BASE_DIR})
            # Also include the v_noabi directories for internal includes
            include_directories(${MONGOCXX_INCLUDE_DIR})
            include_directories(${BSONCXX_INCLUDE_DIR})
            
            if(MONGOCXX_LIBRARY AND BSONCXX_LIBRARY AND MONGOC_LIBRARY AND BSONC_LIBRARY AND UTF8PROC_LIBRARY)
                message(STATUS "Found MongoDB libraries: ${MONGOCXX_LIBRARY}, ${BSONCXX_LIBRARY}, ${MONGOC_LIBRARY}, ${BSONC_LIBRARY}, ${UTF8PROC_LIBRARY}")
                set(MONGODB_LIBRARIES ${MONGOCXX_LIBRARY} ${BSONCXX_LIBRARY} ${MONGOC_LIBRARY} ${BSONC_LIBRARY} ${UTF8PROC_LIBRARY})
                # Add system frameworks for macOS
                if(APPLE)
                    list(APPEND MONGODB_LIBRARIES "-framework Security" "-framework CoreFoundation" "-lresolv")
                endif()
            else()
                message(FATAL_ERROR "MongoDB libraries not found. MONGOCXX_LIBRARY: ${MONGOCXX_LIBRARY}, BSONCXX_LIBRARY: ${BSONCXX_LIBRARY}, MONGOC_LIBRARY: ${MONGOC_LIBRARY}, BSONC_LIBRARY: ${BSONC_LIBRARY}, UTF8PROC_LIBRARY: ${UTF8PROC_LIBRARY}")
            endif()
        else()
            message(FATAL_ERROR "Could not find MongoDB C++ driver headers. MONGOCXX_INCLUDE_DIR: ${MONGOCXX_INCLUDE_DIR}, BSONCXX_INCLUDE_DIR: ${BSONCXX_INCLUDE_DIR}")
        endif()
    endif()  # End of manual finding else block
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
