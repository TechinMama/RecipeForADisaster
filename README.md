# RecipeForADisaster 🍳

A modern C++ recipe management application with MongoDB backend, designed for learning object-oriented programming, database integration, and web service development. Features comprehensive validation, error handling, JSON serialization, and advanced search capabilities.

## 🚀 Features

### Core Functionality
- ✅ **Recipe Management**: Add, view, update, delete recipes
- ✅ **Advanced Search**: Search by title, category, type, or combinations
- ✅ **Pagination**: Efficient handling of large recipe collections
- ✅ **Data Validation**: Comprehensive input validation with custom exceptions
- ✅ **Error Handling**: Detailed error reporting with `OperationResult` structure
- ✅ **JSON Serialization**: Complete JSON API implementation
- ✅ **MongoDB Integration**: Robust database operations with connection pooling
- ✅ **REST API**: Full CRUD operations via HTTP endpoints
- ✅ **Web Interface**: Basic HTML interface with search and management

### Technical Features
- **Object-Oriented Design**: Clean separation of concerns with `recipe` and `recipeManager` classes
- **Cross-Platform**: CMake build system supporting Windows, macOS, and Linux
- **Docker Support**: Containerized development environment with MongoDB
- **Web Framework**: Crow C++ web framework for REST API implementation
- **CORS Support**: Cross-origin resource sharing for web clients
- **Comprehensive Testing**: Integration tests, API tests, and validation tests
- **VS Code Integration**: Custom tasks for efficient development workflow
- **CI/CD Ready**: GitHub Actions multi-platform builds

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Console App   │────│  recipeManager  │────│   MongoDB       │
│   (main.cpp)    │    │  (Business Logic│    │   Database      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Server    │────│  REST API       │────│   Connection    │
│   (web_server) │    │  (Crow Routes)  │    │   Pooling       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │
         ▼                       ▼
┌─────────────────┐    ┌─────────────────┐
│   recipe Class  │    │  Validation &   │
│   (Data Model)  │    │  Error Handling │
└─────────────────┘    └─────────────────┘
```

### Key Components
- **`recipe` class**: Data model with validation and JSON serialization
- **`recipeManager` class**: Business logic layer handling all database operations
- **`web_server` executable**: REST API server built with Crow framework
- **Custom Exceptions**: `ValidationError` and `DatabaseError` for robust error handling
- **JSON API**: Complete REST API with Crow web framework integration

### Design Patterns Used

#### Repository Pattern
The `recipeManager` class implements the Repository pattern, providing a clean abstraction over database operations:
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

#### Factory Pattern
Recipe objects are created through validated constructors, implementing a Factory pattern for object creation:
```cpp
// Validated constructor ensures data integrity
recipe newRecipe("Title", "Ingredients", "Instructions",
                 "4 servings", "20 minutes", "Italian", "Main Course");
```

#### Strategy Pattern
Different search strategies are implemented for various query types:
```cpp
// Title search
auto results = manager.searchRecipes("pasta");

// Category filtering
auto italianRecipes = manager.searchByCategory("Italian");

// Type filtering
auto mainCourses = manager.searchByType("Main Course");
```

### Database Schema

Recipes are stored in MongoDB with the following document structure:
```json
{
  "_id": ObjectId("..."),
  "title": "Chocolate Cake",
  "ingredients": "Flour, Sugar, Cocoa, Eggs, Milk",
  "instructions": "Mix ingredients and bake at 350°F for 30 minutes",
  "servingSize": "8 servings",
  "cookTime": "45 minutes",
  "category": "Dessert",
  "type": "Cake"
}
```

#### Indexes
- **Title Index**: For efficient search operations
- **Category Index**: For category-based filtering
- **Type Index**: For type-based filtering

### Error Handling Strategy

#### Custom Exceptions
```cpp
class ValidationError : public std::runtime_error {
public:
    ValidationError(const std::string& message) : std::runtime_error(message) {}
};

class DatabaseError : public std::runtime_error {
public:
    DatabaseError(const std::string& message) : std::runtime_error(message) {}
};
```

#### Operation Results
Database operations return detailed results:
```cpp
struct OperationResult {
    bool success;
    std::string errorMessage;
    std::string data; // Optional additional data
};
```

## 📋 Recent Updates

### Version 1.1.0 - Web Interface Implementation
- ✅ **Crow Framework Integration**: Complete REST API implementation
- ✅ **Web Server**: Standalone executable with all CRUD operations
- ✅ **MongoDB Driver Update**: Compatibility with MongoDB C++ driver 4.1.2+
- ✅ **API Testing**: Comprehensive test suite for all endpoints
- ✅ **Build System**: Updated CMake configuration for web server target
- ✅ **Documentation**: Complete API documentation and troubleshooting guide

### Key Improvements
- **Lambda Capture Fixes**: Resolved compilation issues with route handlers
- **JSON Serialization**: Proper wvalue handling for API responses
- **Error Handling**: Comprehensive error responses with proper HTTP status codes
- **CORS Support**: Cross-origin resource sharing for web clients
- **Health Checks**: Database connection monitoring endpoint
- **Pagination**: Efficient handling of large datasets

## 📋 Prerequisites

### System Requirements
- **C++ Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: Version 3.16 or higher
- **MongoDB**: Local installation or MongoDB Atlas account
- **MongoDB C++ Driver**: Version 4.1.2+ (latest stable)
- **Crow Framework**: Automatically downloaded via CMake (no manual installation required)

### Installing Dependencies

#### macOS (with Homebrew)
```bash
# Install build tools
brew install cmake gcc

# Install MongoDB C++ driver (version 4.1.2+)
brew install mongo-cxx-driver

# Verify installation
brew list mongo-cxx-driver
```

#### Ubuntu/Debian
```bash
# Install build tools
sudo apt-get update
sudo apt-get install build-essential cmake

# Install MongoDB C++ driver (version 4.1.2+)
sudo apt-get install libmongocxx-dev libbsoncxx-dev

# Verify installation
pkg-config --modversion libmongocxx
```

#### Windows
```bash
# Install MongoDB C++ driver via vcpkg (version 4.1.2+)
vcpkg install mongo-cxx-driver

# Or download from MongoDB website
# https://www.mongodb.com/docs/drivers/cxx/#installation
```

### MongoDB Setup

#### Local MongoDB Installation
```bash
# macOS with Homebrew
brew install mongodb-community
brew services start mongodb-community

# Ubuntu/Debian
sudo apt-get install mongodb
sudo systemctl start mongodb

# Windows
# Download and install from: https://www.mongodb.com/try/download/community
```

#### Docker (Recommended for Development)
```bash
# Quick setup with provided script
./setup-dev.sh

# Or manually
docker run -d -p 27017:27017 --name mongodb mongo:latest
```

## 🛠️ Build Instructions

### Quick Start (Local MongoDB)
```bash
# Clone the repository
git clone https://github.com/TechinMama/RecipeForADisaster.git
cd RecipeForADisaster

# Set MongoDB connection
export MONGODB_URI="mongodb://localhost:27017"

# Build with CMake
cmake -S . -B build/
cmake --build build/

# Run console application
./build/RecipeForADisaster

# Or run web server
./build/web_server
```

### Docker Development Environment
```bash
# Start MongoDB with Docker
./setup-dev.sh

# Or manually with docker-compose
docker-compose up -d

# Build and run
cmake -S . -B build/
cmake --build build/
./build/web_server
```

### Build Options
```bash
# Debug build
cmake -S . -B build-debug/ -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug/

# Release build with optimizations
cmake -S . -B build-release/ -DCMAKE_BUILD_TYPE=Release
cmake --build build-release/

# Build with tests
cmake -S . -B build/ -DBUILD_TESTING=ON
cmake --build build/
ctest --test-dir build/
```

### Building Web Server Only
```bash
# Build only the web server
cmake -S . -B build/
cmake --build build/ --target web_server
./build/web_server
```

## 📖 Usage

### Basic Operations
```cpp
#include "recipeManager.h"

// Initialize with MongoDB URI
mongocxx::instance instance{};
recipeManager manager("mongodb://localhost:27017");

// Add a recipe
recipe pasta("Pasta Carbonara", "Spaghetti, Eggs, Bacon, Cheese",
             "Cook pasta, mix with eggs and cheese, add bacon", "4 servings", "20 minutes", "Italian", "Main Course");
auto result = manager.addRecipe(pasta);
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