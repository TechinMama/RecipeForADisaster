cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17) # Set the C++ standard to C++17

project(RecipeForADisaster)

include_directories(include) # Add the 'include' directory to the include path

file(GLOB SOURCES "src/*.cpp")

add_executable(RecipeForADisaster ${SOURCES}) # Replace 'main.cpp' with your source file(s)
