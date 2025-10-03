# RecipeForADisaster 🍳

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)
[![SQLite](https://img.shields.io/badge/SQLite-3.x-green.svg)](https://www.sqlite.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modern C++ recipe management application with SQLite integration, REST API, and AI-powered recipe generation using Azure OpenAI.

## ✨ Features

- **Complete CRUD Operations** - Create, Read, Update, Delete recipes
- **Advanced Search** - Search by title, category, type, or combinations
- **AI Recipe Generation** - Generate recipes using Azure OpenAI Chat Completion
- **REST API** - Full HTTP API with JSON responses
- **Web Interface** - Clean HTML interface for recipe management
- **Data Validation** - Comprehensive input validation with custom exceptions
- **Cross-Platform** - CMake build system supporting Windows, macOS, and Linux

## 🚀 Quick Start

### Prerequisites
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+
- [vcpkg](https://github.com/microsoft/vcpkg) package manager
- Git

### Setup with vcpkg (Recommended)
```bash
# Clone repository with submodules
git clone --recursive https://github.com/TechinMama/RecipeForADisaster.git
cd RecipeForADisaster

# If you already cloned without --recursive, initialize submodules
git submodule update --init --recursive

# Bootstrap vcpkg
cd vcpkg
./bootstrap-vcpkg.sh  # On macOS/Linux
# or .\bootstrap-vcpkg.bat on Windows

# Build application (vcpkg will automatically install dependencies from vcpkg.json)
cmake -S . -B build/ -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build/

# The application uses a local SQLite database file (recipes.db)
```

### Alternative: System Package Installation
If you prefer system packages instead of vcpkg:

**macOS (with Homebrew):**
```bash
brew install boost sqlite3 cmake
```

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libboost-system-dev libboost-date-time-dev libcurl4-openssl-dev libssl-dev libsqlite3-dev cmake
```

**Windows:**
Use vcpkg as shown above, or install Visual Studio with C++ build tools.

### Docker Compose (Full Stack)
```bash
# Build and run the application (SQLite database is embedded)
docker build -t recipeforadisaster .
docker run -d --name recipeforadisaster -p 8080:8080 recipeforadisaster

# View logs
docker logs -f recipeforadisaster

# Stop application
docker stop recipeforadisaster
```

### Docker Container Deployment (Recommended)
```bash
# Build the container image
docker build -t recipeforadisaster .

# Run the application (SQLite database is embedded in container)
docker run -d --name recipeforadisaster \
  -p 8080:8080 \
  recipeforadisaster
```

### Access
- **Web Interface**: http://localhost:8080
- **API Documentation**: http://localhost:8080/api
- **Health Check**: http://localhost:8080/api/health

## 🤖 AI Recipe Generation

Generate recipes using Azure OpenAI Chat Completion. Describe what you want to cook, and the AI will create complete, structured recipes.

### Setup
1. Create an Azure OpenAI resource in the Azure portal
2. Set environment variables:
```bash
export AZURE_OPENAI_ENDPOINT="https://your-resource-name.openai.azure.com/"
export AZURE_OPENAI_KEY="your-api-key-here"
export AZURE_OPENAI_DEPLOYMENT="gpt-35-turbo"
```

### Usage
- **Web Interface**: Use the "Generate Recipe" form
- **API**: `POST /api/recipes/generate` with a JSON prompt

## 🔐 Secure Credential Management with HashiCorp Vault

For production deployments, use HashiCorp Vault to securely store and retrieve credentials instead of environment variables.

### Vault Setup
1. Install and configure HashiCorp Vault server
2. Create KV secrets engine (default path: `secret/`)
3. Store credentials in Vault:

```bash
# SQLite doesn't require credentials, but you can still store other secrets
vault kv put secret/database/config db_path="/app/data/recipes.db"

# Azure OpenAI credentials
vault kv put secret/ai/azure-openai endpoint="https://your-resource-name.openai.azure.com/" api_key="your-api-key-here" deployment_name="gpt-35-turbo"
```

### Configuration
Set Vault environment variables:
```bash
export VAULT_ADDR="https://your-vault-server.com:8200"
export VAULT_TOKEN="your-vault-token-here"
```

The application will automatically detect Vault configuration and retrieve credentials securely. If Vault is not configured, it falls back to environment variables.

## 📡 API Reference

### Recipes
- `GET /api/recipes` - List all recipes
- `POST /api/recipes` - Create a new recipe
- `PUT /api/recipes/:title` - Update a recipe
- `DELETE /api/recipes/:title` - Delete a recipe
- `GET /api/recipes/search?q=:query` - Search recipes

### AI Features
- `POST /api/recipes/generate` - Generate recipe from prompt
- `GET /api/ai/status` - Check AI service status

## 🧪 Testing

```bash
# Build and run tests
cmake --build build/ --target run_tests

# Run specific test suites
./build/integration_tests
./build/ai_service_tests
```

## � License

MIT License - see [LICENSE](LICENSE) for details.
