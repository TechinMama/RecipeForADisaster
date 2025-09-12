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

# Find MongoDB C++ driver
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# Main application sources (exclude web_server.cpp)
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/web_server.cpp")

# Create main executable
add_executable(RecipeForADisaster ${SOURCES})

# Create web server executable
add_executable(web_server src/web_server.cpp src/recipe.cpp src/recipeManager.cpp)

# Create test executable
add_executable(tests tests/test_integration.cpp src/recipe.cpp src/recipeManager.cpp)

# Link libraries
target_link_libraries(RecipeForADisaster mongo::mongocxx_shared mongo::bsoncxx_shared)
target_link_libraries(web_server Crow::Crow mongo::mongocxx_shared mongo::bsoncxx_shared)
target_link_libraries(tests mongo::mongocxx_shared mongo::bsoncxx_shared)

# Test executable needs the same include directories
target_include_directories(tests PRIVATE include)
target_include_directories(web_server PRIVATE include)

# Add test
add_test(NAME IntegrationTests COMMAND tests)

# Optional: Add a custom target to run tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tests
    COMMENT "Running integration tests"
)
