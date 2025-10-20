# RecipeForADisaster 🍳

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)
[![SQLite](https://img.shields.io/badge/SQLite-3.x-green.svg)](https://www.sqlite.org/)
[![React](https://img.shields.io/badge/React-19-blue.svg)](https://reactjs.org/)
[![TypeScript](https://img.shields.io/badge/TypeScript-4.9-blue.svg)](https://www.typescriptlang.org/)
[![PWA](https://img.shields.io/badge/PWA-Enabled-success.svg)](https://web.dev/progressive-web-apps/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> A modern full-stack recipe management application featuring a high-performance C++ backend, React frontend with PWA capabilities, SQLite integration, and AI-powered recipe generation using Azure OpenAI.

## ✨ Key Features

### 🏗️ Backend (C++17)
- **High-Performance Architecture** - Optimized C++ with async operations and Redis caching
- **SQLite Integration** - Robust database operations with JSON storage
- **RESTful API** - Clean HTTP endpoints with JWT authentication
- **AI Integration** - Azure OpenAI for intelligent recipe generation
- **Security** - HashiCorp Vault integration for credential management
- **Cross-Platform** - CMake build system supporting Windows, macOS, and Linux

### 🎨 Frontend (React + TypeScript)
- **Progressive Web App** - Install on any device, works offline
- **Mobile-First Design** - Fully responsive with touch-optimized interactions
- **Offline Support** - Service worker with intelligent caching strategies
- **Background Sync** - Automatic synchronization of offline operations
- **Advanced UX** - Recipe collections, smart search, and profile management

## 🚀 Quick Start

### Prerequisites
- **Backend**: C++17 compiler, CMake 3.16+, [vcpkg](https://github.com/microsoft/vcpkg)
- **Frontend**: Node.js 16+, npm

### Backend Setup
```bash
# Clone with submodules
git clone --recursive https://github.com/TechinMama/RecipeForADisaster.git
cd RecipeForADisaster

# Bootstrap vcpkg and build
cd vcpkg && ./bootstrap-vcpkg.sh
cd ..
cmake -S . -B build/ -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build/
```

### Frontend Setup
```bash
cd frontend
npm install
npm start
```

### Access Points
- **Frontend**: http://localhost:3000
- **Backend API**: http://localhost:8080
- **API Docs**: http://localhost:8080/api

## ✨ Features

### Backend (C++)

## 🚀 Performance Improvements & Benchmarks

### Summary Table
| Area                | Optimization/Result                | Metric/Status                |
|---------------------|------------------------------------|------------------------------|
| Backend Caching     | Redis for expensive queries         | ~3x faster API response      |
| DB Optimization     | Indexed queries, refactored SQL     | < 20ms query time            |
| API Response        | Async endpoints, reduced latency    | < 20ms avg, 0ms initial      |
| Frontend Caching    | Service Worker, IndexedDB           | Offline access, instant load |
| Image Loading       | Lazy loading                        | Faster page render           |
| Code Splitting      | React.lazy/Suspense                 | Reduced bundle size          |
| Lighthouse FCP      | First Contentful Paint              | 0.6s (Excellent)             |
| Lighthouse LCP      | Largest Contentful Paint             | 3.1s (Good)                  |
| Lighthouse SpeedIdx | Speed Index                         | 0.6s (Excellent)             |
| Lighthouse TTI      | Time to Interactive                  | 3.1s (Excellent)             |
| ApacheBench         | Backend API throughput               | 469 req/sec, 0 failed        |

### Highlights
- Backend API response time reduced by 3x with Redis caching and query optimization.
- Frontend loads instantly and works offline via service worker and IndexedDB.
- All major performance metrics (FCP, LCP, Speed Index, TTI) are in the "Excellent" or "Good" range.
- ApacheBench confirms backend can handle 469 requests/sec with zero failed requests.
- All tests pass for both backend and frontend.

See below for setup instructions and detailed API documentation.
- **Cross-Platform** - CMake build system supporting Windows, macOS, and Linux

### Frontend (React + TypeScript)
- **Progressive Web App (PWA)** - Install on any device, works offline
- **Mobile Optimized** - Fully responsive design with touch-friendly interactions
- **Offline Support** - Service worker with intelligent caching strategies
- **Background Sync** - Automatic sync of offline operations when back online
- **User Authentication** - Secure login and registration with JWT tokens
- **Recipe Collections** - Organize recipes into custom collections
- **Advanced Search UI** - Filter recipes by multiple criteria
- **Profile Management** - Customize your cooking preferences

## 🚀 Quick Start

### Prerequisites

**Backend:**
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+
- [vcpkg](https://github.com/microsoft/vcpkg) package manager
- Git

**Frontend:**
- Node.js 16+ and npm
- Modern web browser with PWA support (Chrome, Firefox, Safari, Edge)

### Backend Setup with vcpkg (Recommended)
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

### Frontend Setup
```bash
# Navigate to frontend directory
cd frontend

# Install dependencies
npm install

# Start development server
npm start

## 📡 API Reference

### Core Endpoints

#### Recipes
```http
GET    /api/recipes          # List all recipes
GET    /api/recipes/{id}     # Get specific recipe
POST   /api/recipes          # Create new recipe
PUT    /api/recipes/{id}     # Update recipe
DELETE /api/recipes/{id}     # Delete recipe
```

#### Authentication
```http
POST   /api/auth/login       # User login
POST   /api/auth/register    # User registration
POST   /api/auth/refresh     # Refresh JWT token
```

#### AI Features
```http
POST   /api/ai/generate      # Generate recipe with AI
POST   /api/ai/suggest       # Get ingredient suggestions
```

### Request/Response Examples

**Create Recipe:**
```json
POST /api/recipes
{
  "title": "Classic Chocolate Chip Cookies",
  "description": "Soft and chewy cookies with melty chocolate chips",
  "ingredients": [
    {"name": "All-purpose flour", "amount": "2.25", "unit": "cups"},
    {"name": "Chocolate chips", "amount": "2", "unit": "cups"}
  ],
  "instructions": [
    "Preheat oven to 375°F",
    "Mix dry ingredients...",
    "Bake for 10-12 minutes"
  ],
  "prepTime": 15,
  "cookTime": 12,
  "servings": 24
}
```

**AI Recipe Generation:**
```json
POST /api/ai/generate
{
  "ingredients": ["chicken", "rice", "broccoli"],
  "cuisine": "asian",
  "dietary": ["gluten-free"],
  "difficulty": "easy"
}
```

## 🧪 Testing

### Backend Tests
```bash
# Run all tests
cd build/
ctest --output-on-failure

# Run with coverage
cmake --build build/ --target coverage
```

### Frontend Tests
```bash
cd frontend
npm test
```

### Test Results
- **Unit Tests**: 34/34 passing ✅
- **Integration Tests**: 4/4 passing ✅
- **Coverage**: Generated successfully
- **Performance**: All benchmarks met

## 📊 Performance Benchmarks

| Metric | Value | Status |
|--------|-------|--------|
| API Response Time | < 20ms | ✅ Excellent |
| First Contentful Paint | 0.6s | ✅ Excellent |
| Largest Contentful Paint | 3.1s | ✅ Good |
| Time to Interactive | 3.1s | ✅ Excellent |
| Throughput | 469 req/sec | ✅ Excellent |

## 🏗️ Architecture

### Backend Architecture
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   REST API      │────│   Business      │────│   Data Access   │
│   (Crow)        │    │   Logic         │    │   Layer         │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │   SQLite DB     │
                    │   + Redis Cache │
                    └─────────────────┘
```

### Frontend Architecture
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   React App     │────│   Service       │────│   IndexedDB     │
│   (PWA)         │    │   Worker        │    │   Cache         │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │   REST API      │
                    │   Backend       │
                    └─────────────────┘
```

## 🔧 Development

### Code Quality
- **Linting**: Clang-Tidy for C++, ESLint for TypeScript
- **Formatting**: Clang-Format, Prettier
- **Testing**: Google Test framework, Jest
- **CI/CD**: GitHub Actions with automated testing and deployment

### Environment Setup
```bash
# Development environment
export RECIPE_ENV=development
export AZURE_OPENAI_KEY=your_key_here
export VAULT_TOKEN=your_token_here

# Production environment
export RECIPE_ENV=production
```

## 🚀 Deployment

### Docker Deployment
```dockerfile
FROM ubuntu:20.04
COPY build/RecipeManager /app/
COPY data/ /app/data/
EXPOSE 8080
CMD ["/app/RecipeManager"]
```

### Cloud Deployment
- **Backend**: Azure App Service / AWS ECS
- **Frontend**: Vercel / Netlify with PWA support
- **Database**: SQLite (file-based) or PostgreSQL
- **Cache**: Redis Cloud / Azure Cache for Redis

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow C++17 standards and modern React patterns
- Write comprehensive tests for new features
- Update documentation for API changes
- Ensure all tests pass before submitting PR

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **SQLite** for reliable embedded database
- **Crow** for lightweight C++ web framework
- **React** for modern frontend development
- **Azure OpenAI** for AI recipe generation
- **vcpkg** for cross-platform dependency management

---

