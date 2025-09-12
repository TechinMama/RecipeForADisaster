# RecipeForADisaster - AI Agent Instructions

## Project Overview
RecipeForADisaster is a C++ console application for managing recipes using MongoDB as the backend database. The application follows object-oriented design with separate concerns for data models, business logic, and data persistence.

## Architecture Patterns

### Core Classes
- **`recipe` class** (`src/recipe.h/.cpp`): Data model with getters/setters for recipe properties (title, ingredients, instructions, servingSize, cookTime, category, type)
- **`recipeManager` class** (`src/recipeManager.h/.cpp`): Business logic layer handling CRUD operations with MongoDB integration
- **Main application** (`src/main.cpp`): Entry point demonstrating usage patterns

### Data Flow
1. **Input**: User interactions through console (planned for future web interface)
2. **Processing**: `recipeManager` handles all database operations via MongoDB C++ driver
3. **Storage**: MongoDB collections store recipe documents with BSON serialization
4. **Output**: Console display of recipe data

### MongoDB Integration Pattern
```cpp
// Always initialize MongoDB instance before creating clients
mongocxx::instance instance{};
recipeManager manager("MONGODB_URI"); // Use environment variable or config

// CRUD operations follow this pattern:
auto collection = db["recipes"];
bsoncxx::builder::stream::document filter{};
filter << "title" << recipeTitle;
```

## Development Workflows

### Build System
- **Primary**: CMake with `cmake --build build/` after `cmake -S . -B build/`
- **Alternative**: Direct clang++ compilation: `clang++ -std=c++17 src/*.cpp -o RecipeManager`
- **Dependencies**: MongoDB C++ driver (mongocxx), bsoncxx libraries

### Environment Setup
```bash
# MongoDB connection requires URI (local or Atlas)
# Set MONGODB_URI environment variable or hardcode for development
export MONGODB_URI="mongodb://localhost:27017"  # or your Atlas URI
```

### Code Organization
- **Headers**: `.h` files in `src/` with include guards (`#ifndef RECIPE_H`)
- **Implementation**: `.cpp` files with matching names
- **Naming**: Classes use camelCase (`recipe`, `recipeManager`), methods follow getter/setter pattern

## Key Conventions

### Error Handling
- **Recipe Validation**: All recipe fields are validated in constructor and setters with descriptive error messages
- **MongoDB Operations**: All database operations use try-catch blocks with `OperationResult` for detailed error reporting
- **Connection Validation**: Database connection is tested during initialization with `isConnected()` method
- **Custom Exceptions**: `ValidationError` for recipe validation, `DatabaseError` for MongoDB operations
- **Safe BSON Parsing**: `safeGetString()` method prevents crashes from malformed database documents

### Database Schema
Recipes stored as MongoDB documents with these fields:
- `title` (string, indexed for search)
- `ingredients` (string)
- `instructions` (string)
- `servingSize` (string)
- `cookTime` (string)
- `category` (string)
- `type` (string)

### Search Implementation
- Regex-based search on `title` field with case-insensitive options
- Returns vector of `recipe` objects matching criteria

## Common Patterns

### Recipe Validation
```cpp
// Constructor validates all fields
recipe newRecipe("Title", "Ingredients", "Instructions",
                 "4 servings", "20 minutes", "Italian", "Main Course");

// Setters also validate input
recipe.setTitle("New Title"); // Throws ValidationError if invalid

// Validation rules:
// - Title: not empty, ≤100 chars, not whitespace-only
// - Ingredients: not empty, ≤1000 chars, not whitespace-only  
// - Instructions: not empty, ≤2000 chars, not whitespace-only
// - Serving Size/Cook Time/Category/Type: not empty, ≤50 chars
```

### Database Operations with Error Handling
```cpp
// Operations return detailed results
auto result = manager.addRecipe(newRecipe);
if (!result.success) {
    std::cout << "Error: " << result.errorMessage << std::endl;
}

// Connection validation
if (!manager.isConnected()) {
    throw DatabaseError("Database connection lost");
}
```

### Adding New Recipes
```cpp
recipe newRecipe("Title", "Ingredients", "Instructions",
                 "4 servings", "20 minutes", "Italian", "Main Course");
manager.addRecipe(newRecipe);
```

### Updating Recipes
```cpp
recipe updated = recipe("Title", "New Ingredients", "New Instructions", ...);
manager.updateRecipe("Title", updated); // Returns OperationResult
```

### Database Connection
Always initialize `mongocxx::instance` once per application lifetime before creating any MongoDB clients.

## Testing Patterns

### Integration Test Structure
```cpp
// Test helper function
void runTest(const std::string& testName, std::function<bool()> testFunc) {
    // Test execution with error handling
}

// Test validation logic
bool testRecipeValidation() {
    // Test valid inputs
    recipe validRecipe("Title", "Ingredients", "Instructions", ...);
    
    // Test invalid inputs (should throw ValidationError)
    try {
        recipe("", "Ingredients", "Instructions", ...); // Empty title
        return false; // Should not reach here
    } catch (const recipe::ValidationError&) {
        // Expected behavior
    }
    return true;
}

// Test database operations
bool testDatabaseOperations() {
    mongocxx::instance instance{};
    recipeManager manager("mongodb://localhost:27017");
    
    // Test CRUD operations with OperationResult
    auto result = manager.addRecipe(testRecipe);
    return result.success;
}
```

## Testing & Validation
- Integration tests in `tests/test_integration.cpp` validate validation logic and error handling
- Manual testing through `main.cpp` examples
- CI/CD via GitHub Actions with multi-platform CMake builds
- Test runner provides detailed pass/fail reporting

### Running Tests
```bash
# Build and run tests
cmake -S . -B build/
cmake --build build/
ctest --test-dir build/ --output-on-failure

# Or use the custom target
cmake --build build/ --target run_tests
```

### Test Coverage
- **Recipe Validation**: Tests all field validation rules and edge cases
- **Database Operations**: Tests CRUD operations with error handling
- **Error Scenarios**: Tests invalid inputs and connection failures
- **Edge Cases**: Tests maximum lengths, special characters, and boundary conditions

## Web Interface Architecture

### Recommended C++ Web Solutions
For C++ applications requiring web interfaces, consider these production-ready options:

#### Crow Framework (Recommended)
Lightweight C++ micro-framework for REST APIs:
```cpp
#include "crow.h"

int main() {
    crow::SimpleApp app;
    
    CROW_ROUTE(app, "/recipes")
    ([](){
        recipeManager manager("MONGODB_URI");
        auto recipes = manager.viewRecipes();
        return crow::json::wvalue{recipes}; // JSON serialization
    });
    
    app.port(18080).multithreaded().run();
}
```

#### Pistache
High-performance REST framework:
- Async request handling
- Built-in JSON support
- Production-ready for high-throughput applications

#### CppCMS
Full-stack web framework:
- Template engine integration
- Session management
- Database ORM capabilities

### Architecture Pattern for Web Interface
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Client    │────│   REST API      │────│   recipeManager │
│   (HTML/JS)     │    │   (Crow/Pistache│    │   (MongoDB)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                       ┌─────────────────┐
                       │   MongoDB       │
                       │   Collections   │
                       └─────────────────┘
```

### Integration Strategy
1. **Keep existing `recipeManager`** - Business logic remains unchanged
2. **Add REST endpoints** - Map CRUD operations to HTTP routes
3. **JSON serialization** - Convert recipe objects to/from JSON
4. **Frontend separation** - Web UI handles presentation, C++ handles data

### JSON Serialization Support
```cpp
// Convert recipe to JSON
recipe pasta("Pasta", "Noodles, Sauce", "Cook pasta", "4", "20min", "Italian", "Main");
std::string json = pasta.toJson();
// {"title":"Pasta","ingredients":"Noodles, Sauce",...}

// Create recipe from JSON
std::string jsonInput = "{\"title\":\"Pizza\",\"ingredients\":\"Dough, Cheese\"}";
recipe pizza = recipe::fromJson(jsonInput);

// JSON API methods in recipeManager
std::string recipesJson = manager.getRecipesJson();
std::string result = manager.addRecipeJson(jsonInput);
```

### Build Considerations
```cmake
# Add to CMakeLists.txt for web framework
find_package(Crow REQUIRED)  # or Pistache, CppCMS
target_link_libraries(RecipeForADisaster Crow::Crow)
```

## CI/CD Workflow

### GitHub Actions Multi-Platform Build
The project uses automated CI/CD with the following matrix:
- **Platforms**: Ubuntu Linux, Windows
- **Compilers**: GCC, Clang, MSVC
- **Build Type**: Release

### Workflow Triggers
- **Push to main branch**: Automatic build and test
- **Pull requests**: Validation before merge

### Efficient Development Workflow
```bash
# Local development cycle
cmake -S . -B build/          # Configure once
cmake --build build/          # Build after changes
./build/RecipeForADisaster    # Run locally

# Before pushing
git add .
git commit -m "feat: add new feature"
git push origin main          # Triggers CI/CD automatically
```

### Build Optimization Tips
- **Incremental builds**: CMake only rebuilds changed files
- **Parallel compilation**: Use `cmake --build build/ -j$(nproc)`
- **Debug builds**: Add `-DCMAKE_BUILD_TYPE=Debug` for development
- **Clean builds**: `rm -rf build/` then reconfigure when needed

### CI/CD Benefits
- **Cross-platform validation**: Ensures code works on Windows/Linux
- **Automated testing**: Catches compilation errors before merge
- **Dependency verification**: Confirms MongoDB driver integration
- **Release readiness**: Release builds are production-ready

### Local Testing Before CI
```bash
# Test build locally before pushing
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/
./build/RecipeForADisaster

# Verify MongoDB connection
export MONGODB_URI="mongodb://localhost:27017"
./build/RecipeForADisaster
```

### Workflow Customization
To add new build configurations, modify `.github/workflows/cmake-multi-platform.yml`:
```yaml
matrix:
  os: [ubuntu-latest, windows-latest, macos-latest]  # Add macOS
  build_type: [Release, Debug]  # Add Debug builds
```

## Deployment Strategies

### Containerization with Docker
For consistent deployment across environments:

```dockerfile
# Dockerfile for RecipeForADisaster
FROM ubuntu:22.04

# Install MongoDB C++ driver dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libmongocxx-dev \
    libbsoncxx-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source and build
WORKDIR /app
COPY . .
RUN cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build/ -j$(nproc)

# Set MongoDB URI environment variable
ENV MONGODB_URI="mongodb://host.docker.internal:27017"

CMD ["./build/RecipeForADisaster"]
```

### Cloud Deployment Options

#### AWS EC2 with MongoDB Atlas
```bash
# EC2 deployment script
#!/bin/bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libmongocxx-dev libbsoncxx-dev

# Build application
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/

# Set environment variables
export MONGODB_URI="mongodb+srv://username:password@cluster.mongodb.net/RecipeManagerDB"

# Run application
./build/RecipeForADisaster
```

#### Azure VM with MongoDB Atlas
```bash
# Azure VM deployment script
#!/bin/bash
# Update system and install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libmongocxx-dev libbsoncxx-dev

# Clone and build the application
git clone https://github.com/TechinMama/RecipeForADisaster.git
cd RecipeForADisaster

# Build application
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/

# Set environment variables for MongoDB Atlas
export MONGODB_URI="mongodb+srv://username:password@cluster.mongodb.net/RecipeManagerDB"

# Run application
./build/RecipeForADisaster
```

#### Azure Container Instances (ACI)
```bash
# Deploy using Azure CLI
az container create \
  --resource-group myResourceGroup \
  --name recipe-manager \
  --image myregistry.azurecr.io/recipe-manager:v1.0 \
  --cpu 1 \
  --memory 1.5 \
  --registry-login-server myregistry.azurecr.io \
  --registry-username myregistry \
  --registry-password mypassword \
  --environment-variables MONGODB_URI="mongodb+srv://username:password@cluster.mongodb.net/RecipeManagerDB" \
  --ports 8080 \
  --protocol TCP
```

#### Azure App Service (for web interface)
```bash
# For web-enabled version with custom build
az webapp create \
  --resource-group myResourceGroup \
  --plan myAppServicePlan \
  --name recipe-manager-app \
  --runtime "NODE|14-lts" \
  --deployment-local-git

# Configure custom deployment script
az webapp config appsettings set \
  --resource-group myResourceGroup \
  --name recipe-manager-app \
  --setting MONGODB_URI="mongodb+srv://username:password@cluster.mongodb.net/RecipeManagerDB"
```

#### Heroku Buildpack (for web interface)
```bash
# For web-enabled version with Crow/Pistache
heroku create recipe-manager-app
heroku buildpacks:add https://github.com/heroku/heroku-buildpack-cmake
git push heroku main
```

### Binary Distribution
```bash
# Create distributable package
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install
cmake --build build/
cmake --install build/

# Package for distribution
tar -czf RecipeForADisaster-v1.0.tar.gz install/
```

## Development Workflow Optimizations

### Advanced Build Configurations

#### Debug Build with Sanitizers
```bash
# Enable address sanitizer for memory debugging
cmake -S . -B build-debug/ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"

cmake --build build-debug/
./build-debug/RecipeForADisaster
```

#### Code Coverage Analysis
```bash
# Generate coverage reports
cmake -S . -B build-coverage/ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="--coverage" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"

cmake --build build-coverage/
./build-coverage/RecipeForADisaster

# Generate coverage report
gcovr -r . --html --html-details -o coverage.html
```

### Development Tools Integration

#### VS Code Tasks for Efficiency
```json
// .vscode/tasks.json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build Debug",
      "type": "shell",
      "command": "cmake",
      "args": ["--build", "build-debug/", "-j$(nproc)"],
      "group": "build",
      "dependsOn": ["Configure Debug"]
    },
    {
      "label": "Configure Debug",
      "type": "shell",
      "command": "cmake",
      "args": ["-S", ".", "-B", "build-debug/", "-DCMAKE_BUILD_TYPE=Debug"]
    }
  ]
}
```

Available VS Code Tasks:
- **Build Debug/Release**: Full CMake build with optimizations
- **Configure Debug/Release**: Set up build directories
- **Clean Build**: Remove all build artifacts
- **Run Application**: Execute the built application
- **Run Tests**: Execute test suite
- **Setup Development Environment**: Run Docker setup script
- **Start/Stop MongoDB**: Docker Compose management

#### Pre-commit Hooks
```bash
#!/bin/bash
# .git/hooks/pre-commit

# Run build before commit
cmake --build build/
if [ $? -ne 0 ]; then
    echo "Build failed. Fix errors before committing."
    exit 1
fi

# Check for common issues
if grep -r "TODO" src/; then
    echo "Warning: TODO comments found"
fi
```

### Performance Optimization Workflow

#### Profiling with Valgrind
```bash
# Memory profiling
valgrind --tool=memcheck ./build/RecipeForADisaster

# Callgrind for performance profiling
valgrind --tool=callgrind ./build/RecipeForADisaster
kcachegrind callgrind.out.*
```

#### Compiler Optimization Flags
```cmake
# In CMakeLists.txt for production builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
```

### Database Development Workflow

#### Local MongoDB Setup with Docker
```bash
# Quick setup with provided script
./setup-dev.sh

# Or manually with docker-compose
docker-compose up -d

# Connect with application
export MONGODB_URI="mongodb://localhost:27017"
./build/RecipeForADisaster
```

#### Environment Configuration
```bash
# Copy example environment file
cp .env.example .env

# Set MongoDB URI
export MONGODB_URI="mongodb://localhost:27017"
```

#### Database Migration Scripts
```cpp
// migration.cpp - Handle schema updates
void runMigrations(mongocxx::database& db) {
    // Create indexes for performance
    auto collection = db["recipes"];
    collection.create_index(bsoncxx::builder::stream::document{}
        << "title" << 1 << bsoncxx::builder::stream::finalize);

    // Add new fields to existing documents
    // Migration logic here
}
```

### Testing Workflow Enhancements

#### Integration Test Setup
```cpp
// test_integration.cpp
#include <cassert>
#include "recipeManager.h"

void testFullWorkflow() {
    mongocxx::instance instance{};
    recipeManager manager("mongodb://localhost:27017");
    
    // Test complete CRUD cycle
    recipe testRecipe("Test", "Ingredients", "Instructions", 
                     "4", "20min", "Test", "Main");
    
    assert(manager.addRecipe(testRecipe));
    auto recipes = manager.viewRecipes();
    assert(!recipes.empty());
    
    // Cleanup
    manager.deleteRecipe("Test");
}
```

#### Automated Testing with CMake
```cmake
# Add to CMakeLists.txt
enable_testing()

add_executable(tests test_integration.cpp src/recipe.cpp src/recipeManager.cpp)
target_link_libraries(tests mongocxx bsoncxx)

add_test(NAME IntegrationTests COMMAND tests)
```</content>
<parameter name="filePath">/Users/alexandramccoy/RecipeForADisaster/.github/copilot-instructions.md