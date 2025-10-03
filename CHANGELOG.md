# Changelog

All notable changes to **RecipeForADisaster** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-10-03

### 🚀 Major Updates

**RecipeForADisaster v1.1.0** brings significant improvements to build reliability, dependency management, and deployment capabilities.

### ✨ Added

#### Build System & Dependencies
- **Standalone ASIO Support**: Updated Crow framework to v1.2.0 with standalone ASIO compatibility
- **Enhanced Static Linking**: Complete MongoDB static linking with all transitive dependencies
- **Improved Dependency Management**: Added nlohmann/json and zlib to build dependencies
- **Cross-Platform Compatibility**: Verified builds on Windows, macOS, and Linux

#### Infrastructure Improvements
- **Docker Integration**: Updated container build with new vcpkg dependencies
- **CI/CD Pipeline**: Enhanced GitHub Actions workflow for multi-platform builds
- **Repository Management**: Added vcpkg/ directory to .gitignore

### 🔧 Changed

#### Technical Updates
- **Crow Framework**: Upgraded from v1.0+5 to v1.2.0 (standalone ASIO)
- **MongoDB Integration**: Improved static linking with proper header detection
- **Build Configuration**: Enhanced CMake configuration for better dependency resolution
- **ASIO Dependencies**: Replaced Boost.ASIO with standalone ASIO library

### 🐛 Fixed

#### Build & Linking Issues
- **MongoDB Static Linking**: Resolved undefined symbol errors in production builds
- **BSONCXX Header Detection**: Fixed header path detection from `document.hpp` to `json.hpp`
- **ASIO Library Detection**: Corrected CMake configuration for standalone ASIO
- **Include Path Configuration**: Added proper v_noabi include directory support

### 🔄 Migration Notes
- **Breaking Changes**: Requires MongoDB C++ driver with v_noabi headers
- **New Dependencies**: Applications using static linking now require additional system libraries
- **Build System**: Updated CMake configuration may require clean rebuilds

### 📋 Dependencies Updated
- **Crow**: v1.0+5 → v1.2.0 (standalone ASIO)
- **ASIO**: Added standalone ASIO library
- **Build Tools**: Enhanced vcpkg and CMake configurations

---

## [1.0.0] - 2025-10-02

### 🎉 Initial Release

**RecipeForADisaster** is a modern C++ recipe management application with MongoDB integration, REST API, and AI-powered recipe generation using Azure OpenAI.

### ✨ Added

#### Core Features
- **Complete Recipe Management System**
  - Create, Read, Update, Delete (CRUD) operations for recipes
  - Recipe data model with title, ingredients, instructions, serving size, cook time, category, and type
  - Comprehensive data validation with custom exceptions
  - Advanced search functionality by title, category, type, or combinations
  - Paginated recipe listings for better performance

- **AI-Powered Recipe Generation**
  - Integration with Azure OpenAI Chat Completion API
  - Generate complete recipes from natural language descriptions
  - Support for generating multiple recipe suggestions
  - Configurable AI parameters (temperature, max tokens, etc.)
  - Input validation and error handling for AI requests

- **REST API**
  - Full HTTP REST API with JSON responses
  - CORS support for web applications
  - Comprehensive error handling and status codes
  - Health check endpoint (`/api/health`)
  - API documentation accessible through web interface

- **Web Interface**
  - Clean, modern HTML/CSS/JavaScript interface
  - Recipe browsing and management
  - AI recipe generation form
  - Real-time search functionality
  - Responsive design for desktop and mobile

#### Security & Infrastructure
- **HashiCorp Vault Integration**
  - Secure credential management for production deployments
  - Support for MongoDB and Azure OpenAI credentials
  - Graceful fallback to environment variables when Vault unavailable
  - Token-based authentication for Vault access

- **Build System**
  - CMake-based build system supporting multiple platforms
  - Cross-platform support (Windows, macOS, Linux)
  - Automated dependency management with FetchContent
  - GitHub Actions CI/CD pipeline for multi-platform builds

#### Technical Implementation
- **Database Layer**
  - MongoDB integration with C++ driver
  - Connection pooling and error handling
  - BSON-based data serialization
  - Database health monitoring

- **Web Framework**
  - Crow C++ web framework for HTTP server
  - Middleware support for CORS and error handling
  - JSON request/response handling
  - Static file serving for web interface

- **Testing Infrastructure**
  - Unit tests for core components
  - Integration tests for database operations
  - AI service testing framework
  - Vault integration tests

### 🔧 Technical Details

#### Dependencies
- **Core Libraries**
  - Crow v1.0+5 (Web framework)
  - MongoDB C++ Driver 4.1.2+
  - nlohmann/json v3.11.2 (JSON handling)
  - Azure Core SDK 1.10.3 (Azure integration)

- **System Dependencies**
  - C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
  - CMake 3.16+
  - libcurl (HTTP client)
  - OpenSSL (TLS support)

#### API Endpoints
```
GET    /api/recipes                    - List all recipes (paginated)
POST   /api/recipes                    - Create new recipe
PUT    /api/recipes/:title             - Update existing recipe
DELETE /api/recipes/:title             - Delete recipe
GET    /api/recipes/search?q=:query    - Search recipes
GET    /api/recipes/categories/:cat    - Get recipes by category
GET    /api/recipes/types/:type        - Get recipes by type
POST   /api/recipes/generate           - Generate AI recipe
GET    /api/ai/status                  - Check AI service status
GET    /api/health                     - System health check
```

### 📚 Documentation
- Comprehensive README with setup instructions
- API documentation accessible via web interface
- Environment configuration examples
- Development setup guide
- Docker integration for easy MongoDB setup

### 🐛 Known Issues
- Vault integration is partially implemented (requires completion for production use)
- No authentication/authorization system for API access
- No rate limiting on API endpoints
- Requires external MongoDB instance
- Azure OpenAI service requires paid subscription

### 🔄 Migration Notes
- First release - no migration needed
- Environment variables remain compatible
- Database schema is established in this release

### 👥 Contributors
- **Alexandra McCoy** - Primary developer and maintainer

### 📄 License
- BSD 2-Clause License

---

## Development Notes

This release represents the culmination of extensive development work including:
- C++ application architecture and design patterns
- Database integration and query optimization
- AI service integration and error handling
- Web development with modern JavaScript
- DevOps and CI/CD pipeline setup
- Security considerations and credential management
- Cross-platform build system implementation
- Comprehensive testing strategy

The application is production-ready for basic recipe management use cases, with room for future enhancements in security, scalability, and additional features.