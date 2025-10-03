#!/bin/bash

# RecipeForADisaster Local Development Setup Script

set -e

echo "🍳 Setting up RecipeForADisaster local development environment..."
echo

# Check if Docker is installed (optional - SQLite doesn't require it)
if command -v docker &> /dev/null; then
    echo "🐳 Docker detected - you can use it for additional services if needed"
else
    echo "ℹ️  Docker not detected - not required for SQLite-based development"
fi

echo
echo "🎉 Local development environment is ready!"
echo "   SQLite database will be created automatically at: recipes.db"
echo
echo "� Available commands:"
echo "   • Build: ./build.sh"
echo "   • Run: ./build/RecipeForADisaster"
echo "   • Clean: rm -rf build recipes.db"
echo
echo "� To reset database:"
echo "   rm recipes.db"