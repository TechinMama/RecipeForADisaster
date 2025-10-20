# RecipeForADisaster 🍳

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org/)
[![SQLite](https://img.shields.io/badge/SQLite-3.x-green.svg)](https://www.sqlite.org/)
[![React](https://img.shields.io/badge/React-19-blue.svg)](https://reactjs.org/)
[![TypeScript](https://img.shields.io/badge/TypeScript-4.9-blue.svg)](https://www.typescriptlang.org/)
[![PWA](https://img.shields.io/badge/PWA-Enabled-success.svg)](https://web.dev/progressive-web-apps/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modern full-stack recipe management application with C++ backend, React frontend, SQLite integration, and AI-powered recipe generation using Azure OpenAI. Features Progressive Web App (PWA) capabilities for offline access and mobile optimization.

## ✨ Features

### Backend (C++)
- **Complete CRUD Operations** - Create, Read, Update, Delete recipes
- **Advanced Search** - Search by title, category, type, or combinations
- **AI Recipe Generation** - Generate recipes using Azure OpenAI Chat Completion
- **REST API** - Full HTTP API with JSON responses
- **Data Validation** - Comprehensive input validation with custom exceptions
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

# Build for production (includes PWA features)
npm run build
```

### Access the Application
- **Frontend Development**: http://localhost:3000
- **Backend API**: http://localhost:8080
- **API Documentation**: http://localhost:8080/api
- **Health Check**: http://localhost:8080/api/health

## 📱 Progressive Web App Features

### PWA Installation
The app can be installed on any device:
- **Desktop**: Click the install button in the address bar (Chrome, Edge) or use the install prompt
- **iOS**: Safari → Share → Add to Home Screen
- **Android**: Chrome → Menu → Install App

### Offline Functionality
- **Offline Access**: View cached recipes without internet connection
- **Background Sync**: Operations performed offline automatically sync when back online
- **Smart Caching**: Service worker intelligently caches recipes and static assets
- **Offline Indicators**: Visual feedback when offline with pending operations counter

### Mobile Optimization
- **Responsive Design**: Optimized layouts for all screen sizes (desktop, tablet, mobile)
- **Touch-Friendly**: Large tap targets and gesture support
- **Safe Area Support**: Proper handling of notched devices (iPhone X+)
- **Performance**: Optimized assets and lazy loading for fast mobile experience

### Docker Container Deployment
```bash
# Build the container image
docker build -t recipeforadisaster .

# Run the application (SQLite database is embedded in container)
docker run -d --name recipeforadisaster \
  -p 8080:8080 \
  recipeforadisaster

# View logs
docker logs -f recipeforadisaster

# Stop application
docker stop recipeforadisaster

# For full stack with frontend
docker-compose up -d
```

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

### Authentication
- `POST /api/auth/register` - Register new user
- `POST /api/auth/login` - Login user
- `GET /api/auth/user` - Get current user
- `PUT /api/auth/user` - Update user profile
- `POST /api/auth/logout` - Logout user

### Recipes
- `GET /api/recipes` - List all recipes
- `POST /api/recipes` - Create a new recipe
- `PUT /api/recipes/:title` - Update a recipe
- `DELETE /api/recipes/:title` - Delete a recipe
- `GET /api/recipes/search?q=:query` - Search recipes
- `GET /api/recipes/category/:category` - Get recipes by category
- `GET /api/recipes/type/:type` - Get recipes by type

### Collections
- `GET /api/collections` - List user's collections
- `POST /api/collections` - Create new collection
- `PUT /api/collections/:id` - Update collection
- `DELETE /api/collections/:id` - Delete collection
- `POST /api/collections/:id/recipes` - Add recipe to collection
- `DELETE /api/collections/:id/recipes/:recipeId` - Remove recipe from collection

### AI Features
- `POST /api/recipes/generate` - Generate recipe from prompt
- `GET /api/ai/status` - Check AI service status

## 🧪 Testing

### Frontend Testing
```bash
cd frontend

# Run all tests
npm test

# Run tests with coverage
npm test -- --coverage --watchAll=false

# Run specific test suite
npm test -- --testNamePattern="Authentication"
```

**Test Coverage:**
- Component tests with React Testing Library
- Integration tests for authentication flow
- API mocking for isolated testing
- PWA component compatibility with jsdom
- **Current Status**: 41/41 tests passing ✅

### Backend Testing
```bash
# Build and run tests
cmake --build build/ --target run_tests

# Run specific test suites
./build/integration_tests
./build/ai_service_tests
```

## 🌐 Browser Compatibility

### Fully Supported (PWA Features)
- ✅ Chrome/Edge 80+ (Desktop & Mobile)
- ✅ Firefox 90+ (Desktop & Mobile)
- ✅ Safari 14+ (Desktop & iOS)
- ✅ Samsung Internet 12+

### Core Features (No PWA)
- ✅ All modern browsers with ES6+ support
- ✅ Internet Explorer 11 (with polyfills)

## 🔐 Environment Variables

### Backend
```bash
# Azure OpenAI Configuration
export AZURE_OPENAI_ENDPOINT="https://your-resource-name.openai.azure.com/"
export AZURE_OPENAI_KEY="your-api-key-here"
export AZURE_OPENAI_DEPLOYMENT="gpt-35-turbo"

# Optional: HashiCorp Vault
export VAULT_ADDR="https://your-vault-server.com:8200"
export VAULT_TOKEN="your-vault-token-here"
```

### Frontend (Optional)
```bash
# API Base URL (defaults to http://localhost:8080/api)
REACT_APP_API_URL=http://localhost:8080/api

# Push Notifications (optional, for future enhancement)
REACT_APP_VAPID_PUBLIC_KEY=your-vapid-public-key
```

## 📦 Project Structure

```
RecipeForADisaster/
├── src/                      # C++ backend source
│   ├── main.cpp             # Application entry point
│   ├── recipeManager*.cpp   # Recipe management logic
│   └── aiService.cpp        # Azure OpenAI integration
├── frontend/                 # React frontend
│   ├── public/
│   │   ├── manifest.json    # PWA manifest
│   │   ├── sw.js           # Service worker
│   │   └── index.html      # HTML template
│   ├── src/
│   │   ├── components/     # React components
│   │   ├── contexts/       # React contexts (Auth)
│   │   ├── hooks/          # Custom React hooks
│   │   ├── services/       # API services
│   │   └── types/          # TypeScript types
│   └── package.json        # Frontend dependencies
├── tests/                   # Backend tests
├── build/                   # Build output
├── vcpkg/                   # Dependency management
├── CMakeLists.txt          # CMake configuration
└── README.md               # This file
```

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📚 Additional Documentation

- [Production Deployment Guide](PRODUCTION_DEPLOYMENT.md)
- [Functional Testing Guide](FUNCTIONAL_TESTING_GUIDE.md)
- [Quick Start Guide](QUICKSTART_GUIDE.md)
- [Changelog](CHANGELOG.md)

## 🙏 Acknowledgments

- Built with [React](https://reactjs.org/) and [TypeScript](https://www.typescriptlang.org/)
- Backend powered by C++ and [Boost](https://www.boost.org/)
- Database management with [SQLite](https://www.sqlite.org/)
- AI capabilities via [Azure OpenAI](https://azure.microsoft.com/en-us/products/ai-services/openai-service)
- PWA features following [web.dev best practices](https://web.dev/progressive-web-apps/)

## 📞 Support

For issues and questions:
- Open an [Issue](https://github.com/TechinMama/RecipeForADisaster/issues)
- Check existing [Discussions](https://github.com/TechinMama/RecipeForADisaster/discussions)

---

Made with ❤️ and 🍳 by the RecipeForADisaster team
