cmake_minimum_required(VERSION 3.10)

project(RecipeManager)

add_executable(my_executable src/main.cpp) # Replace 'main.cpp' with your source file(s)

set(CMAKE_CXX_STANDARD 17) # Set the C++ standard to C++17

include_directories(include) # Add the 'include' directory to the include path

#add_library(my_library STATIC src/my_library.cpp) # Create a static library