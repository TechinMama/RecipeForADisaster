#!/bin/bash

# Simple Makefile-based build script for RecipeForADisaster
# Alternative to CMake if MongoDB integration issues persist

set -e

echo "🔨 Building RecipeForADisaster with Make..."

# Detect OS and set paths
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    MONGODB_INCLUDE="/opt/homebrew/include"
    MONGODB_LIB="/opt/homebrew/lib"
    LIBS="-lmongocxx -lbsoncxx -lcurl -lssl -lcrypto -lz"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    MONGODB_INCLUDE="/usr/local/include"
    MONGODB_LIB="/usr/local/lib"
    LIBS="-lmongocxx -lbsoncxx -lcurl -lssl -lcrypto -lz -pthread"
else
    echo "❌ Unsupported OS: $OSTYPE"
    exit 1
fi

# Compiler flags
CXXFLAGS="-std=c++17 -I$MONGODB_INCLUDE -I./src -O2"
LDFLAGS="-L$MONGODB_LIB"

echo "📁 Include path: $MONGODB_INCLUDE"
echo "📚 Library path: $MONGODB_LIB"

# Create build directory
mkdir -p build
cd build

# Compile all source files
echo "⚙️  Compiling..."
g++ $CXXFLAGS $LDFLAGS \
    ../src/recipeManager.cpp \
    ../src/recipe.cpp \
    ../src/aiService.cpp \
    ../src/vaultService.cpp \
    ../src/common_utils.cpp \
    ../src/main.cpp \
    -o RecipeForADisaster \
    $LIBS

echo "✅ Build complete! Run ./build/RecipeForADisaster"