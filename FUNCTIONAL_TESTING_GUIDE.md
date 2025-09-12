# Functional Testing Guide for RecipeForADisaster

## 📋 Pre-Testing Checklist

### Environment Verification
- [ ] macOS/Linux/Windows system
- [ ] CMake 3.16+ installed (`cmake --version`)
- [ ] C++ compiler installed (GCC 9+, Clang 10+, or MSVC 2019+)
- [ ] MongoDB C++ driver 4.1.2+ installed
- [ ] Docker installed (for MongoDB container)
- [ ] Python 3.x installed (for API testing)

### Directory Structure Check
- [ ] Project cloned to `/Users/alexandramccoy/RecipeForADisaster`
- [ ] All source files present in `src/` directory
- [ ] `CMakeLists.txt` present in root
- [ ] `test_api.py` present in root (API testing script)

---

## 🧪 Functional Testing Steps

### Step 1: Environment Setup and Verification

#### 1.1 Verify System Requirements
```bash
# Check CMake version
cmake --version

# Check C++ compiler
clang++ --version  # or g++ --version

# Check MongoDB driver (macOS with Homebrew)
brew list mongo-cxx-driver

# Check Docker
docker --version
```

**Expected Output:**
- CMake version 3.16 or higher
- C++ compiler available
- MongoDB driver installed
- Docker available

#### 1.2 Start MongoDB Container
```bash
# Navigate to project directory
cd /Users/alexandramccoy/RecipeForADisaster

# Start MongoDB with Docker
docker run -d --name recipeforadisaster_mongodb -p 27017:27017 mongo:latest

# Verify MongoDB is running
docker ps | grep mongodb
```

**Expected Output:**
- MongoDB container running on port 27017

---

### Step 2: Build System Testing

#### 2.1 Clean Build Directory
```bash
# Remove any existing build artifacts
rm -rf build/

# Create fresh build directory
mkdir build && cd build/
```

#### 2.2 Configure with CMake
```bash
# From project root
cmake -S . -B build/

# Check for any configuration errors
echo "CMake configuration completed with exit code: $?"
```

**Expected Output:**
- No CMake errors
- Build files generated in `build/` directory

#### 2.3 Build All Targets
```bash
# Build all executables
cmake --build build/

# Verify executables were created
ls -la build/
```

**Expected Output:**
- `RecipeForADisaster` executable
- `web_server` executable
- `tests` executable (if built with tests)

#### 2.4 Build with Tests Enabled
```bash
# Clean and rebuild with tests
rm -rf build/
cmake -S . -B build/ -DBUILD_TESTING=ON
cmake --build build/
```

**Expected Output:**
- All targets build successfully
- No compilation errors

---

### Step 3: Console Application Testing

#### 3.1 Set Environment Variables
```bash
# Set MongoDB connection string
export MONGODB_URI="mongodb://localhost:27017"

# Verify environment variable
echo $MONGODB_URI
```

#### 3.2 Test Console Application Launch
```bash
# Run console application
./build/RecipeForADisaster
```

**Expected Output:**
- Application starts without crashes
- MongoDB connection successful message
- Interactive menu displayed

#### 3.3 Test Basic Recipe Operations
```bash
# Run console app and test adding a recipe
./build/RecipeForADisaster
# Select option 1 (Add Recipe)
# Enter test data:
# Title: Test Pasta
# Ingredients: Noodles, Sauce, Cheese
# Instructions: Cook pasta, add sauce
# Serving Size: 4 servings
# Cook Time: 20 minutes
# Category: Italian
# Type: Main Course
```

**Expected Output:**
- Recipe added successfully
- No validation errors

#### 3.4 Test Search Functionality
```bash
# Test search in console application
./build/RecipeForADisaster
# Select option 2 (Search Recipes)
# Enter search term: "Pasta"
```

**Expected Output:**
- Test recipe found and displayed
- Search results formatted correctly

---

### Step 4: Web Server Testing

#### 4.1 Start Web Server
```bash
# Ensure MongoDB is running
docker ps | grep mongodb

# Start web server in background
./build/web_server &
WEB_SERVER_PID=$!

# Wait for server to start
sleep 3

# Verify server is running
ps aux | grep web_server
```

**Expected Output:**
- Web server process running
- Console output shows successful MongoDB connection
- Server listening on port 8080

#### 4.2 Test Health Endpoint
```bash
# Test health check
curl -s http://localhost:8080/api/health | python3 -m json.tool
```

**Expected Output:**
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

#### 4.3 Test Web Interface
```bash
# Test main web interface
curl -s http://localhost:8080/
```

**Expected Output:**
- HTML content returned
- No server errors

---

### Step 5: API Endpoint Testing

#### 5.1 Test GET All Recipes
```bash
# Get all recipes
curl -s http://localhost:8080/api/recipes | python3 -m json.tool
```

**Expected Output:**
```json
{
    "success": true,
    "data": {
        "page": 1,
        "pageSize": 10,
        "totalCount": 0,
        "totalPages": 0,
        "recipes": []
    }
}
```

#### 5.2 Test POST Add Recipe
```bash
# Add a new recipe
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{
    "title": "API Test Recipe",
    "ingredients": "Test ingredients",
    "instructions": "Test instructions",
    "servingSize": "4 servings",
    "cookTime": "30 minutes",
    "category": "Test",
    "type": "Main Course"
  }' | python3 -m json.tool
```

**Expected Output:**
```json
{
    "success": true,
    "data": {
        "message": "Recipe added successfully",
        "title": "API Test Recipe"
    }
}
```

#### 5.3 Test GET Recipes After Adding
```bash
# Verify recipe was added
curl -s http://localhost:8080/api/recipes | python3 -m json.tool
```

**Expected Output:**
- Recipe appears in the list
- Pagination data updated

#### 5.4 Test Search Functionality
```bash
# Search for the added recipe
curl -s "http://localhost:8080/api/recipes/search?q=API" | python3 -m json.tool
```

**Expected Output:**
- API test recipe found in search results

#### 5.5 Test Category Filtering
```bash
# Filter by category
curl -s http://localhost:8080/api/recipes/categories/Test | python3 -m json.tool
```

**Expected Output:**
- Test category recipes returned

#### 5.6 Test PUT Update Recipe
```bash
# Update the recipe
curl -X PUT http://localhost:8080/api/recipes/API%20Test%20Recipe \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Updated API Test Recipe",
    "ingredients": "Updated ingredients",
    "instructions": "Updated instructions",
    "servingSize": "6 servings",
    "cookTime": "45 minutes",
    "category": "Updated",
    "type": "Test"
  }' | python3 -m json.tool
```

**Expected Output:**
```json
{
    "success": true,
    "data": {
        "message": "Recipe updated successfully",
        "oldTitle": "API Test Recipe",
        "newTitle": "Updated API Test Recipe"
    }
}
```

#### 5.7 Test DELETE Recipe
```bash
# Delete the recipe
curl -X DELETE http://localhost:8080/api/recipes/Updated%20API%20Test%20Recipe | python3 -m json.tool
```

**Expected Output:**
```json
{
    "success": true,
    "data": {
        "message": "Recipe deleted successfully",
        "title": "Updated API Test Recipe"
    }
}
```

---

### Step 6: Automated API Testing

#### 6.1 Run Python Test Script
```bash
# Run comprehensive API tests
python3 test_api.py
```

**Expected Output:**
- All tests pass (4/4)
- No connection errors
- API endpoints responding correctly

#### 6.2 Run Unit Tests
```bash
# Run C++ unit tests
cd build/
ctest --output-on-failure
```

**Expected Output:**
- All tests pass
- No assertion failures

---

### Step 7: Error Scenario Testing

#### 7.1 Test Invalid JSON
```bash
# Test malformed JSON
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{"invalid": json}' 2>/dev/null | python3 -m json.tool
```

**Expected Output:**
```json
{
    "success": false,
    "error": "Invalid JSON body"
}
```

#### 7.2 Test Missing Required Fields
```bash
# Test incomplete recipe data
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{"title": "Incomplete Recipe"}' | python3 -m json.tool
```

**Expected Output:**
- Validation error with specific message

#### 7.3 Test Non-existent Recipe
```bash
# Try to update non-existent recipe
curl -X PUT http://localhost:8080/api/recipes/NonExistentRecipe \
  -H "Content-Type: application/json" \
  -d '{"title": "Test"}' | python3 -m json.tool
```

**Expected Output:**
```json
{
    "success": false,
    "error": "Recipe not found",
    "code": 404
}
```

#### 7.4 Test Database Connection Failure
```bash
# Stop MongoDB container
docker stop recipeforadisaster_mongodb

# Test API endpoint
curl -s http://localhost:8080/api/health | python3 -m json.tool

# Restart MongoDB
docker start recipeforadisaster_mongodb
```

**Expected Output:**
- Health check shows database_connected: false
- API gracefully handles connection failure

---

### Step 8: Performance Testing

#### 8.1 Test Pagination
```bash
# Test with different page sizes
curl -s "http://localhost:8080/api/recipes?page=1&pageSize=5" | python3 -m json.tool
```

**Expected Output:**
- Correct pagination data
- Appropriate number of results

#### 8.2 Test Large Dataset Handling
```bash
# Add multiple recipes via script (if available)
# Test search performance
curl -s "http://localhost:8080/api/recipes/search?q=test" | python3 -m json.tool
```

**Expected Output:**
- Fast response times
- Correct results

---

### Step 9: Integration Testing

#### 9.1 Test Full Workflow
```bash
# 1. Add recipe via API
curl -X POST http://localhost:8080/api/recipes \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Integration Test",
    "ingredients": "Test ingredients",
    "instructions": "Test instructions",
    "servingSize": "4 servings",
    "cookTime": "30 minutes",
    "category": "Integration",
    "type": "Test"
  }'

# 2. Verify in console application
./build/RecipeForADisaster
# Search for "Integration"

# 3. Update via API
curl -X PUT http://localhost:8080/api/recipes/Integration%20Test \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Updated Integration Test",
    "ingredients": "Updated ingredients",
    "instructions": "Updated instructions",
    "servingSize": "6 servings",
    "cookTime": "45 minutes",
    "category": "Integration",
    "type": "Test"
  }'

# 4. Delete via API
curl -X DELETE http://localhost:8080/api/recipes/Updated%20Integration%20Test
```

**Expected Output:**
- All operations successful
- Data consistency between API and console app

---

### Step 10: Cleanup and Final Verification

#### 10.1 Stop Services
```bash
# Stop web server
kill $WEB_SERVER_PID

# Stop MongoDB container
docker stop recipeforadisaster_mongodb
docker rm recipeforadisaster_mongodb
```

#### 10.2 Final Build Verification
```bash
# Clean rebuild
rm -rf build/
cmake -S . -B build/
cmake --build build/

# Verify all executables
ls -la build/RecipeForADisaster build/web_server
```

#### 10.3 Documentation Check
```bash
# Verify README is up to date
head -20 README.md

# Check API documentation
grep -A 10 "API Endpoints" README.md
```

---

## 📊 Test Results Summary

### Record Your Results:
- [ ] Environment setup: PASS/FAIL
- [ ] Build system: PASS/FAIL
- [ ] Console application: PASS/FAIL
- [ ] Web server startup: PASS/FAIL
- [ ] Health endpoint: PASS/FAIL
- [ ] CRUD operations: PASS/FAIL
- [ ] Search functionality: PASS/FAIL
- [ ] Error handling: PASS/FAIL
- [ ] Performance: PASS/FAIL
- [ ] Integration: PASS/FAIL

### Common Issues and Solutions:
- **MongoDB connection fails**: Check Docker container status
- **Build fails**: Verify MongoDB driver installation
- **API returns 500**: Check MongoDB connection and logs
- **Web server won't start**: Verify port 8080 is available
- **Tests fail**: Ensure all dependencies are installed

### Performance Benchmarks:
- Build time: ____ seconds
- API response time: ____ ms
- Search query time: ____ ms
- Database connection time: ____ ms

---

## 🎯 Quick Test Commands

```bash
# One-line environment check
cmake --version && clang++ --version && docker --version

# Quick build and test
rm -rf build/ && cmake -S . -B build/ && cmake --build build/

# Start full stack
docker run -d -p 27017:27017 mongo:latest && sleep 3 && export MONGODB_URI="mongodb://localhost:27017" && ./build/web_server &

# Test all endpoints
curl http://localhost:8080/api/health && curl http://localhost:8080/api/recipes

# Run all tests
cd build/ && ctest --output-on-failure
```

**Happy Testing! 🧪✨**