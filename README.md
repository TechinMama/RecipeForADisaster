# RecipeForADisaster 🍳

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)
[![MongoDB](https://img.shields.io/badge/MongoDB-4.0+-green.svg)](https://www.mongodb.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> A modern C++ recipe management application featuring MongoDB integration, REST API, and comprehensive validation. Perfect for learning object-oriented programming, database operations, and web service development.

## 📋 Table of Contents

- [✨ Features](#-features)
- [🏗️ Architecture](#️-architecture)
- [🚀 Quick Start](#-quick-start)
- [📦 Installation](#-installation)
- [🛠️ Build Instructions](#️-build-instructions)
- [📖 Usage](#-usage)
- [🌐 API Reference](#-api-reference)
- [🧪 Testing](#-testing)
- [🔧 Configuration](#-configuration)
- [📚 Documentation](#-documentation)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

---

## ✨ Features

### 🎯 Core Functionality
- ✅ **Complete CRUD Operations** - Create, Read, Update, Delete recipes
- ✅ **Advanced Search** - Search by title, category, type, or combinations
- ✅ **Pagination Support** - Efficient handling of large recipe collections
- ✅ **Data Validation** - Comprehensive input validation with custom exceptions
- ✅ **Error Handling** - Detailed error reporting with structured responses
- ✅ **JSON Serialization** - Complete JSON API implementation
- ✅ **MongoDB Integration** - Robust database operations with connection pooling

### 🌐 Web & API Features
- ✅ **REST API** - Full CRUD operations via HTTP endpoints
- ✅ **Web Interface** - Clean HTML interface with search and management
- ✅ **CORS Support** - Cross-origin resource sharing for web clients
- ✅ **Health Monitoring** - Database connection and system health checks
- ✅ **Crow Framework** - Modern C++ web framework integration

### 🛠️ Technical Features
- ✅ **Object-Oriented Design** - Clean separation with `recipe` and `recipeManager` classes
- ✅ **Cross-Platform** - CMake build system supporting Windows, macOS, and Linux
- ✅ **Docker Support** - Containerized development environment
- ✅ **Comprehensive Testing** - Integration tests, API tests, and validation tests
- ✅ **VS Code Integration** - Custom tasks for efficient development workflow
- ✅ **CI/CD Ready** - GitHub Actions multi-platform builds

---

## 🏗️ Architecture

```mermaid
graph TB
    A[Console App<br/>main.cpp] --> B[recipeManager<br/>Business Logic]
    C[Web Server<br/>web_server.cpp] --> B
    B --> D[MongoDB<br/>Database]

    E[REST API<br/>Crow Routes] --> B
    F[Web Interface<br/>HTML/CSS] --> E

    G[recipe Class<br/>Data Model] --> B
    H[Validation<br/>Error Handling] --> G

    style A fill:#e1f5fe
    style C fill:#e1f5fe
    style B fill:#fff3e0
    style D fill:#e8f5e8
    style E fill:#f3e5f5
    style G fill:#fff3e0
    style H fill:#ffebee
```

### Key Components

| Component | Description | Technology |
|-----------|-------------|------------|
| **recipe** | Data model with validation and JSON serialization | C++ Class |
| **recipeManager** | Business logic layer handling database operations | C++ Class |
| **web_server** | REST API server with Crow framework | Executable |
| **Console App** | Interactive command-line interface | Executable |
| **MongoDB** | Document database for recipe storage | Database |

### Design Patterns

#### 🏭 Repository Pattern
```cpp
class recipeManager {
public:
    OperationResult addRecipe(const recipe& recipe);
    OperationResult updateRecipe(const std::string& title, const recipe& updatedRecipe);
    OperationResult deleteRecipe(const std::string& title);
    std::vector<recipe> viewRecipes();
    std::vector<recipe> searchRecipes(const std::string& criteria);
};
```

#### 🎯 Factory Pattern
```cpp
// Validated constructor ensures data integrity
recipe newRecipe("Title", "Ingredients", "Instructions",
                 "4 servings", "20 minutes", "Italian", "Main Course");
```

#### 🔄 Strategy Pattern
```cpp
// Multiple search strategies
auto titleResults = manager.searchRecipes("pasta");
auto categoryResults = manager.searchByCategory("Italian");
auto typeResults = manager.searchByType("Main Course");
```

---

## 🚀 Quick Start

### ⚡ One-Command Setup
```bash
# Clone and setup
git clone https://github.com/TechinMama/RecipeForADisaster.git
cd RecipeForADisaster

# Start MongoDB (Docker)
docker run -d --name recipeforadisaster_mongodb -p 27017:27017 mongo:latest --noauth

# Set environment and build
export MONGODB_URI="mongodb://localhost:27017"
cmake -S . -B build/ && cmake --build build/

# Start web server
./build/web_server
```

### 🌐 Access Your App
- **Web Interface**: http://localhost:8080
- **API Documentation**: http://localhost:8080/api
- **Health Check**: http://localhost:8080/api/health

---

## 📦 Installation

### 📋 Prerequisites

| Component | Version | Installation |
|-----------|---------|--------------|
| **C++ Compiler** | GCC 9+, Clang 10+, MSVC 2019+ | System package manager |
| **CMake** | 3.16+ | `brew install cmake` / `apt install cmake` |
| **MongoDB C++ Driver** | 4.1.2+ | `brew install mongo-cxx-driver` |
| **Docker** | Latest | `brew install docker` (macOS) |

### 🖥️ Platform-Specific Setup

#### macOS (Homebrew)
```bash
# Install build tools
brew install cmake gcc

# Install MongoDB C++ driver
brew install mongo-cxx-driver

# Verify installation
brew list mongo-cxx-driver
pkg-config --modversion libmongocxx
```

#### Ubuntu/Debian
```bash
# Install build tools
sudo apt-get update
sudo apt-get install build-essential cmake

# Install MongoDB C++ driver
sudo apt-get install libmongocxx-dev libbsoncxx-dev

# Verify installation
pkg-config --modversion libmongocxx
```

#### Windows
```powershell
# Install MongoDB C++ driver via vcpkg
vcpkg install mongo-cxx-driver

# Or download from MongoDB website
# https://www.mongodb.com/docs/drivers/cxx/#installation
```

### 🐳 MongoDB Setup

#### Docker (Recommended)
```bash
# Quick setup
./setup-dev.sh

# Manual setup
docker run -d --name recipeforadisaster_mongodb \
  -p 27017:27017 \
  -v mongodb_data:/data/db \
  mongo:latest --noauth
```

#### Local Installation
```bash
# macOS
brew install mongodb-community
brew services start mongodb-community

# Ubuntu
sudo apt-get install mongodb
sudo systemctl start mongodb
```

---

## 🛠️ Build Instructions

### 📦 Standard Build
```bash
# Configure
cmake -S . -B build/

# Build all targets
cmake --build build/

# Verify executables
ls -la build/
# RecipeForADisaster  web_server  tests
```

### 🎯 Build Options

| Build Type | Command | Use Case |
|------------|---------|----------|
| **Release** | `cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release` | Production deployment |
| **Debug** | `cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug` | Development with debugging |
| **Testing** | `cmake -S . -B build/ -DBUILD_TESTING=ON` | Run test suite |

### 🎨 Build Targets

```bash
# Build specific targets
cmake --build build/ --target RecipeForADisaster  # Console app only
cmake --build build/ --target web_server         # Web server only
cmake --build build/ --target tests              # Tests only

# Clean build
rm -rf build/
cmake -S . -B build/ && cmake --build build/
```

### 🐳 Docker Build
```bash
# Build with Docker Compose
docker-compose up --build

# Or build specific service
docker-compose build web_server
docker-compose up web_server
```

---

## 📖 Usage

### 💻 Console Application

```bash
# Start the console app
export MONGODB_URI="mongodb://localhost:27017"
./build/RecipeForADisaster
```

#### Interactive Menu
```
Recipe Manager Menu:
1. Add Recipe
2. View Recipes
3. Search Recipes
4. Update Recipe
5. Delete Recipe
6. Exit

Enter your choice (1-6): 1
```

#### Programmatic Usage
```cpp
#include "recipeManager.h"

// Initialize
mongocxx::instance instance{};
recipeManager manager("mongodb://localhost:27017");

// Add recipe
recipe pasta("Pasta Carbonara", "Spaghetti, Eggs, Bacon, Cheese",
             "Cook pasta, mix with eggs and cheese, add bacon",
             "4 servings", "20 minutes", "Italian", "Main Course");

auto result = manager.addRecipe(pasta);
if (result.success) {
    std::cout << "✅ Recipe added successfully!" << std::endl;
} else {
    std::cout << "❌ Error: " << result.errorMessage << std::endl;
}

// Search recipes
auto recipes = manager.searchRecipes("pasta");
std::cout << "Found " << recipes.size() << " recipes" << std::endl;
```

### 🌐 Web API Usage

```bash
# Start web server
./build/web_server

# API is now available at http://localhost:8080
```

#### JavaScript Example
```javascript
// Add a recipe
fetch('/api/recipes', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    title: "Chocolate Cake",
    ingredients: "Flour, Sugar, Cocoa, Eggs, Milk",
    instructions: "Mix ingredients and bake at 350°F for 30 minutes",
    servingSize: "8 servings",
    cookTime: "45 minutes",
    category: "Dessert",
    type: "Cake"
  })
})
.then(response => response.json())
.then(data => console.log(data));

// Get all recipes
fetch('/api/recipes')
  .then(response => response.json())
  .then(data => console.log(data));
```

#### Python Example
```python
import requests

BASE_URL = "http://localhost:8080"

# Add recipe
recipe_data = {
    "title": "Pizza Margherita",
    "ingredients": "Dough, Tomato Sauce, Mozzarella, Basil",
    "instructions": "Bake dough with toppings",
    "servingSize": "4 servings",
    "cookTime": "25 minutes",
    "category": "Italian",
    "type": "Main Course"
}

response = requests.post(f"{BASE_URL}/api/recipes", json=recipe_data)
print(response.json())

# Get recipes
response = requests.get(f"{BASE_URL}/api/recipes")
recipes = response.json()
print(f"Found {len(recipes['data']['recipes'])} recipes")
```

---

## 🌐 API Reference

### 📍 Base URL
```
http://localhost:8080/api
```

### 🔗 Endpoints

| Method | Endpoint | Description | Response |
|--------|----------|-------------|----------|
| `GET` | `/health` | System health check | `200 OK` |
| `GET` | `/recipes` | Get all recipes (paginated) | `200 OK` |
| `GET` | `/recipes/search?q={query}` | Search recipes by title | `200 OK` |
| `GET` | `/recipes/categories/{category}` | Filter by category | `200 OK` |
| `GET` | `/recipes/types/{type}` | Filter by type | `200 OK` |
| `POST` | `/recipes` | Create new recipe | `200 OK` / `400 Bad Request` |
| `PUT` | `/recipes/{title}` | Update existing recipe | `200 OK` / `404 Not Found` |
| `DELETE` | `/recipes/{title}` | Delete recipe | `200 OK` / `404 Not Found` |

### 📄 Request/Response Format

#### Recipe Object
```json
{
  "title": "string (required)",
  "ingredients": "string (required)",
  "instructions": "string (required)",
  "servingSize": "string (required)",
  "cookTime": "string (required)",
  "category": "string (required)",
  "type": "string (required)"
}
```

#### Success Response
```json
{
  "success": true,
  "data": {
    "message": "Operation completed successfully",
    "title": "Recipe Title"
  }
}
```

#### Error Response
```json
{
  "success": false,
  "error": "Error description",
  "code": 400
}
```

### 🔍 Query Parameters

#### Pagination
```bash
GET /api/recipes?page=1&pageSize=10
```

#### Search
```bash
GET /api/recipes/search?q=pasta
GET /api/recipes/categories/Italian
GET /api/recipes/types/Main%20Course
```

### 📊 Response Examples

#### Health Check
```bash
curl http://localhost:8080/api/health
```
```json
{
  "success": true,
  "data": {
    "status": "healthy",
    "database_connected": true,
    "timestamp": 1726172800
  }
}
```

#### Get Recipes
```bash
curl http://localhost:8080/api/recipes
```
```json
{
  "success": true,
  "data": {
    "page": 1,
    "pageSize": 10,
    "totalCount": 25,
    "totalPages": 3,
    "recipes": [...]
  }
}
```

#### Add Recipe
```bash
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{"title": "Test Recipe", "ingredients": "Test", "instructions": "Test", "servingSize": "4", "cookTime": "30min", "category": "Test", "type": "Test"}'
```
```json
{
  "success": true,
  "data": {
    "message": "Recipe added successfully",
    "title": "Test Recipe"
  }
}
```

---

## 🧪 Testing

### 🏃 Run Tests

```bash
# Build with tests
cmake -S . -B build/ -DBUILD_TESTING=ON
cmake --build build/

# Run all tests
ctest --test-dir build/ --output-on-failure

# Run specific test
ctest --test-dir build/ -R IntegrationTests
```

### 🧪 Test Categories

| Test Type | Description | Command |
|-----------|-------------|---------|
| **Integration** | Database operations and error handling | `ctest -R IntegrationTests` |
| **Validation** | Input validation and edge cases | `ctest -R ValidationTests` |
| **API** | REST endpoints and web server | `python3 test_api.py` |
| **Performance** | Search and pagination benchmarks | `ctest -R PerformanceTests` |

### 🐍 API Testing Script

```bash
# Run comprehensive API tests
python3 test_api.py

# Expected output:
# Testing RecipeForADisaster Web API...
# ==================================================
#
# Testing Health Check:
# ✅ PASSED
#
# Testing Get Recipes:
# ✅ PASSED
#
# Testing Add Recipe:
# ✅ PASSED
#
# Testing Search Recipes:
# ✅ PASSED
#
# Results: 4/4 tests passed
```

### 🔍 Manual Testing

```bash
# Test console application
./build/RecipeForADisaster

# Test web server
./build/web_server &
curl http://localhost:8080/api/health
curl http://localhost:8080/api/recipes

# Test with sample data
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{"title": "Test", "ingredients": "Test", "instructions": "Test", "servingSize": "4", "cookTime": "30min", "category": "Test", "type": "Test"}'
```

---

## 🔧 Configuration

### 🌍 Environment Variables

| Variable | Description | Default | Required |
|----------|-------------|---------|----------|
| `MONGODB_URI` | MongoDB connection string | `mongodb://localhost:27017` | Yes |
| `PORT` | Web server port | `8080` | No |

### 📁 Configuration Files

#### `.env` (Optional)
```bash
# MongoDB Configuration
MONGODB_URI=mongodb://localhost:27017
DB_NAME=RecipeManagerDB

# Web Server Configuration
PORT=8080
HOST=0.0.0.0

# Development Settings
DEBUG=true
LOG_LEVEL=info
```

#### `CMakeLists.txt` Build Configuration
```cmake
# Build options
option(BUILD_TESTING "Build test suite" ON)
option(BUILD_WEB_SERVER "Build web server" ON)

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MongoDB settings
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
```

### 🐳 Docker Configuration

#### `docker-compose.yml`
```yaml
version: '3.8'
services:
  mongodb:
    image: mongo:latest
    ports:
      - "27017:27017"
    volumes:
      - mongodb_data:/data/db
    command: --noauth

  web_server:
    build: .
    ports:
      - "8080:8080"
    environment:
      - MONGODB_URI=mongodb://mongodb:27017
    depends_on:
      - mongodb

volumes:
  mongodb_data:
```

---

## 📚 Documentation

### 📖 Available Documentation

| Document | Description | Location |
|----------|-------------|----------|
| **Quick Start Guide** | Complete setup and usage guide | `QUICKSTART_GUIDE.md` |
| **Functional Testing** | Comprehensive testing procedures | `FUNCTIONAL_TESTING_GUIDE.md` |
| **API Documentation** | Detailed API reference | `README.md` (this file) |
| **Architecture Guide** | System design and patterns | `.github/copilot-instructions.md` |

### 🔗 Useful Links

- **GitHub Repository**: https://github.com/TechinMama/RecipeForADisaster
- **MongoDB C++ Driver**: https://www.mongodb.com/docs/drivers/cxx/
- **Crow Framework**: https://github.com/CrowCpp/Crow
- **CMake Documentation**: https://cmake.org/documentation/

### 📋 Code Examples

#### Basic Recipe Management
```cpp
// Include headers
#include "recipe.h"
#include "recipeManager.h"

// Initialize MongoDB
mongocxx::instance instance{};

// Create recipe manager
recipeManager manager("mongodb://localhost:27017");

// Create and add recipe
recipe newRecipe("Title", "Ingredients", "Instructions",
                 "4 servings", "20 minutes", "Italian", "Main Course");
auto result = manager.addRecipe(newRecipe);
```

#### Error Handling
```cpp
try {
    recipe invalidRecipe("", "Ingredients", "Instructions", ...); // Empty title
} catch (const recipe::ValidationError& e) {
    std::cout << "Validation error: " << e.what() << std::endl;
} catch (const recipeManager::DatabaseError& e) {
    std::cout << "Database error: " << e.what() << std::endl;
}
```

#### JSON Operations
```cpp
// Convert to JSON
std::string json = recipe.toJson();

// Parse from JSON
recipe fromJson = recipe::fromJson(jsonString);

// API operations
std::string recipesJson = manager.getRecipesJson();
std::string addResult = manager.addRecipeJson(jsonInput);
```

---

## 🤝 Contributing

### 🚀 Getting Started
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes and add tests
4. Run the test suite: `ctest --test-dir build/`
5. Commit your changes: `git commit -m 'Add amazing feature'`
6. Push to the branch: `git push origin feature/amazing-feature`
7. Open a Pull Request

### 📝 Development Guidelines

#### Code Style
- Use C++17 features and standard library
- Follow Google C++ Style Guide
- Use meaningful variable and function names
- Add documentation for public APIs

#### Testing
- Write tests for new features
- Ensure all tests pass before submitting PR
- Test both console and web interfaces
- Include integration tests for database operations

#### Commit Messages
```
feat: add new recipe search functionality
fix: resolve MongoDB connection timeout issue
docs: update API documentation
test: add validation tests for recipe creation
```

### 🐛 Reporting Issues
- Use GitHub Issues for bug reports
- Include steps to reproduce
- Provide system information (OS, compiler, MongoDB version)
- Attach relevant log files

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2024 RecipeForADisaster

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

## 🙏 Acknowledgments

- **Crow Framework** - Modern C++ web framework
- **MongoDB C++ Driver** - Official MongoDB driver for C++
- **CMake** - Cross-platform build system
- **Docker** - Containerization platform

---

## 📞 Support

### 💬 Getting Help
- **Documentation**: Check `QUICKSTART_GUIDE.md` and `FUNCTIONAL_TESTING_GUIDE.md`
- **Issues**: [GitHub Issues](https://github.com/TechinMama/RecipeForADisaster/issues)
- **Discussions**: [GitHub Discussions](https://github.com/TechinMama/RecipeForADisaster/discussions)

### 🔧 Troubleshooting
- **Build Issues**: Ensure MongoDB C++ driver is properly installed
- **Runtime Errors**: Check MongoDB connection and environment variables
- **API Problems**: Verify web server is running on correct port

---

## 📚 Code Examples

#### Basic Recipe Management
```cpp
// Include headers
#include "recipe.h"
#include "recipeManager.h"

// Initialize MongoDB
mongocxx::instance instance{};

// Create recipe manager
recipeManager manager("mongodb://localhost:27017");

// Create and add recipe
recipe newRecipe("Title", "Ingredients", "Instructions",
                 "4 servings", "20 minutes", "Italian", "Main Course");
auto result = manager.addRecipe(newRecipe);

if (result.success) {
    std::cout << "Recipe added successfully!" << std::endl;
} else {
    std::cout << "Error: " << result.errorMessage << std::endl;
}

// Search recipes
auto recipes = manager.searchRecipes("pasta");
for (const auto& r : recipes) {
    std::cout << r.getTitle() << std::endl;
}

// Advanced search with pagination
auto paginatedResult = manager.getRecipesPaginated(1, 10);
std::cout << "Page " << paginatedResult.page << " of " << paginatedResult.totalPages << std::endl;
```

### JSON API (Web Interface Ready)
```cpp
// Convert recipe to JSON
std::string json = pasta.toJson();

// Create recipe from JSON
std::string jsonInput = R"(
{
    "title": "Pizza Margherita",
    "ingredients": "Dough, Tomato Sauce, Mozzarella, Basil",
    "instructions": "Bake dough with toppings",
    "servingSize": "4 servings",
    "cookTime": "25 minutes",
    "category": "Italian",
    "type": "Main Course"
}
)";
recipe pizza = recipe::fromJson(jsonInput);

// JSON API methods in recipeManager
std::string recipesJson = manager.getRecipesJson();
std::string addResult = manager.addRecipeJson(jsonInput);
```
    "ingredients": "Dough, Tomato Sauce, Mozzarella, Basil",
    "instructions": "Bake dough with toppings",
    "servingSize": "4 servings",
    "cookTime": "25 minutes",
    "category": "Italian",
    "type": "Main Course"
}
)";
recipe pizza = recipe::fromJson(jsonInput);

// JSON API methods in recipeManager
std::string recipesJson = manager.getRecipesJson();
std::string addResult = manager.addRecipeJson(jsonInput);
```

## 🧪 Testing

### Run All Tests
```bash
# Build with tests enabled
cmake -S . -B build/ -DBUILD_TESTING=ON
cmake --build build/

# Run tests
ctest --test-dir build/ --output-on-failure

# Or use the custom target
cmake --build build/ --target run_tests
```

### Test Structure
- **Integration Tests**: Validate database operations and error handling
- **Validation Tests**: Test all input validation rules and edge cases
- **Performance Tests**: Benchmark search and pagination operations
- **API Tests**: Test REST endpoints and web server functionality

### Running Tests

#### Build and Run All Tests
```bash
# Build with tests enabled
cmake -S . -B build/ -DBUILD_TESTING=ON
cmake --build build/

# Run all tests
ctest --test-dir build/ --output-on-failure

# Or use the custom target
cmake --build build/ --target run_tests
```

#### API Testing with Python Script
```bash
# Use the provided test script
python3 test_api.py

# Or test individual endpoints
curl http://localhost:8080/api/health
curl http://localhost:8080/api/recipes
```

#### Manual Testing
```bash
# Test with sample data
export MONGODB_URI="mongodb://localhost:27017"
./build/RecipeForADisaster

# Test web server
./build/web_server
# Then visit: http://localhost:8080
```

### Test Coverage
- ✅ **Recipe Validation**: Constructor validation, setter validation, edge cases
- ✅ **Database Operations**: CRUD operations, error handling, connection management
- ✅ **Search Functionality**: Title search, category filtering, type filtering
- ✅ **Pagination**: Page size limits, total count accuracy
- ✅ **API Endpoints**: All REST routes, JSON serialization, error responses
- ✅ **Web Interface**: Basic HTML interface, CORS support

## 🔧 Troubleshooting

### Common Build Issues

#### MongoDB C++ Driver Installation Problems
```bash
# macOS: Check if driver is properly installed
brew list mongo-cxx-driver

# If missing, reinstall
brew uninstall mongo-cxx-driver
brew install mongo-cxx-driver

# Ubuntu/Debian: Check version
pkg-config --modversion libmongocxx

# If outdated, update package list and reinstall
sudo apt-get update
sudo apt-get install --reinstall libmongocxx-dev libbsoncxx-dev
```

#### CMake Configuration Issues
```bash
# Clean build directory and reconfigure
rm -rf build/
cmake -S . -B build/

# Check CMake version
cmake --version

# Ensure MongoDB driver is found
cmake -S . -B build/ -DCMAKE_FIND_DEBUG_MODE=ON
```

#### Compilation Errors with Lambda Captures
If you encounter lambda capture errors in `web_server.cpp`:
```cpp
// Error: variable 'createErrorResponse' cannot be implicitly captured
// Solution: Add to lambda capture list
[&manager, &createErrorResponse, &createSuccessResponse](const crow::request& req) {
    // route handler code
}
```

### Database Connection Issues

#### MongoDB Connection Failed
```bash
# Check if MongoDB is running
docker ps | grep mongo

# Start MongoDB if not running
docker run -d -p 27017:27017 --name mongodb mongo:latest

# Test connection
mongosh --eval "db.runCommand({ping: 1})"

# Set environment variable
export MONGODB_URI="mongodb://localhost:27017"
```

#### Invalid MongoDB URI
```bash
# Common URI formats:
export MONGODB_URI="mongodb://localhost:27017"
export MONGODB_URI="mongodb://username:password@localhost:27017/database"
export MONGODB_URI="mongodb+srv://username:password@cluster.mongodb.net/database"
```

### Web Server Issues

#### Port Already in Use
```bash
# Find process using port 8080
lsof -i :8080

# Kill the process
kill -9 <PID>

# Or use a different port by modifying web_server.cpp
```

#### CORS Issues in Browser
If you encounter CORS errors when testing the API:
- The web server includes CORS middleware by default
- Check browser console for specific error messages
- Ensure you're accessing from `http://localhost:8080`

#### Web Server Won't Start
```bash
# Check MongoDB connection first
export MONGODB_URI="mongodb://localhost:27017"
./build/web_server

# If connection fails, verify MongoDB is running
docker ps
docker logs <mongodb_container_id>
```

### Testing Issues

#### Tests Fail Due to Missing Dependencies
```bash
# Ensure all dependencies are installed
brew install mongo-cxx-driver  # macOS
sudo apt-get install libmongocxx-dev libbsoncxx-dev  # Ubuntu

# Rebuild with tests
cmake -S . -B build/ -DBUILD_TESTING=ON
cmake --build build/
```

#### API Tests Can't Connect
```bash
# Ensure web server is running in background
./build/web_server &

# Wait a moment, then test
curl http://localhost:8080/api/health

# Or use the Python test script
python3 test_api.py
```

### Performance Issues

#### Slow Database Queries
- Ensure proper indexes are created on frequently queried fields
- Consider pagination for large result sets
- Check MongoDB logs for slow query warnings

#### High Memory Usage
- Monitor MongoDB connection pool size
- Implement proper connection cleanup
- Use pagination to limit result set sizes

### Development Environment Issues

#### VS Code IntelliSense Problems
```json
// Add to .vscode/settings.json
{
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/src",
        "${workspaceFolder}/include"
    ],
    "C_Cpp.default.compilerPath": "/usr/bin/clang++"
}
```

#### Git Issues
```bash
# Reset build artifacts
git clean -fdx
git reset --hard HEAD

# Rebuild from scratch
cmake -S . -B build/
cmake --build build/
```

## 🌐 Web Interface (Crow Framework)

The project now includes a complete web interface built with the Crow C++ web framework, providing a modern REST API and interactive web frontend.

### Web Server Features
- ✅ **REST API**: Complete CRUD operations via HTTP endpoints
- ✅ **Interactive Frontend**: HTML/CSS/JavaScript interface
- ✅ **Search & Pagination**: Advanced search with pagination support
- ✅ **CORS Support**: Cross-origin resource sharing enabled
- ✅ **Error Handling**: Comprehensive error responses and validation
- ✅ **Health Checks**: Database connection monitoring

### Quick Start with Web Interface

```bash
# Build the web server
cmake -S . -B build/
cmake --build build/

# Set MongoDB connection
export MONGODB_URI="mongodb://localhost:27017"

# Start the web server
./build/web_server
```

The web interface will be available at: **http://localhost:8080**

### API Endpoints

#### Recipes Management
```http
GET    /api/recipes                    # Get all recipes (paginated)
GET    /api/recipes/search?q=query    # Search recipes
GET    /api/recipes/categories/:cat   # Get recipes by category
GET    /api/recipes/types/:type       # Get recipes by type
POST   /api/recipes                   # Add new recipe
PUT    /api/recipes/:title            # Update recipe
DELETE /api/recipes/:title            # Delete recipe
```

#### System
```http
GET    /api/health                    # Health check
GET    /                             # Web interface
```

### API Usage Examples

#### Get All Recipes
```bash
curl http://localhost:8080/api/recipes
```

#### Search Recipes
```bash
curl "http://localhost:8080/api/recipes/search?q=pasta"
```

#### Add New Recipe
```bash
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Chocolate Cake",
    "ingredients": "Flour, Sugar, Cocoa, Eggs, Milk",
    "instructions": "Mix ingredients and bake at 350°F for 30 minutes",
    "servingSize": "8 servings",
    "cookTime": "45 minutes",
    "category": "Dessert",
    "type": "Cake"
  }'
```

#### Get Recipes by Category
```bash
curl http://localhost:8080/api/recipes/categories/Italian
```

### Web Interface Features

The built-in web interface provides:
- **Recipe Browser**: View all recipes with pagination
- **Search Functionality**: Real-time search across recipe titles
- **Add Recipes**: Form-based recipe creation
- **Delete Recipes**: One-click recipe removal
- **Responsive Design**: Works on desktop and mobile devices

### Docker Deployment

#### Build Docker Image
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git libmongocxx-dev libbsoncxx-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone and build Crow
RUN git clone https://github.com/CrowCpp/Crow.git && \
    cd Crow && mkdir build && cd build && \
    cmake .. && make && make install

COPY . /app
WORKDIR /app

RUN cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build/

EXPOSE 8080
ENV MONGODB_URI="mongodb://host.docker.internal:27017"
CMD ["./build/web_server"]
```

#### Run with Docker Compose
```yaml
version: '3.8'
services:
  mongodb:
    image: mongo:latest
    ports:
      - "27017:27017"
    volumes:
      - mongodb_data:/data/db

  web:
    build: .
    ports:
      - "8080:8080"
    environment:
      - MONGODB_URI=mongodb://mongodb:27017
    depends_on:
      - mongodb

volumes:
  mongodb_data:
```

### Production Deployment

#### System Requirements
- **C++ Compiler**: GCC 9+ or Clang 10+
- **MongoDB**: 4.0+ (local or MongoDB Atlas)
- **RAM**: Minimum 512MB
- **Storage**: 100MB for application + database storage

#### Security Considerations
- Set strong MongoDB credentials
- Use HTTPS in production (consider reverse proxy like Nginx)
- Implement authentication/authorization for sensitive operations
- Validate all input data on both client and server side

#### Performance Optimization
- Use connection pooling for MongoDB
- Implement caching for frequently accessed recipes
- Enable gzip compression for API responses
- Configure appropriate thread pool size for concurrent requests

## 🔧 Development

### VS Code Integration
The project includes custom VS Code tasks for efficient development:

- **Build Debug/Release**: Full CMake builds with optimizations
- **Run Tests**: Execute test suite with detailed output
- **Setup Environment**: Docker-based development setup
- **Clean Build**: Remove build artifacts

### Code Organization
```
src/
├── main.cpp              # Application entry point
├── recipe.h/.cpp         # Data model with validation
└── recipeManager.h/.cpp  # Business logic and database operations

tests/
├── test_integration.cpp  # Database and validation tests
└── test_validation.cpp   # Standalone validation tests

.vscode/
└── tasks.json           # VS Code development tasks
```

### Environment Variables
```bash
# MongoDB connection
export MONGODB_URI="mongodb://localhost:27017"
# Or for MongoDB Atlas
export MONGODB_URI="mongodb+srv://username:password@cluster.mongodb.net/RecipeManagerDB"

# Optional: Logging level
export LOG_LEVEL="DEBUG"
```

## 📚 API Reference

### recipe Class
```cpp
// Constructor with validation
recipe::recipe(const std::string& title, const std::string& ingredients,
               const std::string& instructions, const std::string& servingSize,
               const std::string& cookTime, const std::string& category,
               const std::string& type)

// JSON serialization
std::string toJson() const
static recipe fromJson(const std::string& json)

// Validation
void validateTitle(const std::string& title)
void validateIngredients(const std::string& ingredients)
// ... other validation methods
```

### recipeManager Class
```cpp
// CRUD operations
OperationResult addRecipe(const recipe& recipe)
OperationResult updateRecipe(const std::string& title, const recipe& updatedRecipe)
OperationResult deleteRecipe(const std::string& title)
std::vector<reipe> viewRecipes()

// Search operations
std::vector<reipe> searchRecipes(const std::string& criteria)
std::vector<reipe> searchByCategory(const std::string& category)
std::vector<reipe> searchByType(const std::string& type)
std::vector<reipe> searchByCategoryAndType(const std::string& category, const std::string& type)

// Pagination
PaginatedResult getRecipesPaginated(int page, int pageSize)
PaginatedResult searchRecipesPaginated(const std::string& criteria, int page, int pageSize)

// JSON API
std::string getRecipesJson()
std::string addRecipeJson(const std::string& json)
std::string updateRecipeJson(const std::string& title, const std::string& json)

// Connection management
bool isConnected() const
```

## 🤝 Contributing

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/amazing-feature`
3. **Commit** your changes: `git commit -m 'Add amazing feature'`
4. **Push** to the branch: `git push origin feature/amazing-feature`
5. **Open** a Pull Request

### Development Guidelines
- Follow the existing code style and naming conventions
- Add comprehensive tests for new features
- Update documentation for API changes
- Ensure all tests pass before submitting PR
- Use meaningful commit messages

### Code Style
- **Classes**: PascalCase (`RecipeManager`, `ValidationError`)
- **Methods**: camelCase (`addRecipe()`, `searchRecipes()`)
- **Variables**: camelCase (`recipeTitle`, `searchCriteria`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_TITLE_LENGTH`)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- MongoDB C++ Driver documentation
- CMake best practices
- C++ community for object-oriented design patterns
- Open source contributors

## 📞 Support

For questions or issues:
- Open an issue on GitHub
- Check the [Wiki](https://github.com/TechinMama/RecipeForADisaster/wiki) for detailed guides
- Review the [copilot-instructions.md](.github/copilot-instructions.md) for AI-assisted development

---

**Happy Cooking! 🍳✨**