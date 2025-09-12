#!/bin/bash

# RecipeForADisaster Local Development Setup Script

set -e

echo "🍳 Setting up RecipeForADisaster local development environment..."
echo

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Please install Docker first."
    echo "   Visit: https://docs.docker.com/get-docker/"
    exit 1
fi

# Check if Docker Compose is available
if ! command -v docker-compose &> /dev/null && ! docker compose version &> /dev/null; then
    echo "❌ Docker Compose is not available. Please install Docker Compose."
    exit 1
fi

echo "🐳 Starting MongoDB with Docker Compose..."
if command -v docker-compose &> /dev/null; then
    docker-compose up -d
else
    docker compose up -d
fi

echo
echo "⏳ Waiting for MongoDB to be ready..."
sleep 5

# Test MongoDB connection
echo "🔍 Testing MongoDB connection..."
if docker exec recipeforadisaster_mongodb mongo --eval "db.adminCommand('ismaster')" &> /dev/null; then
    echo "✅ MongoDB is running successfully!"
else
    echo "❌ MongoDB connection failed. Please check the logs:"
    echo "   docker logs recipeforadisaster_mongodb"
    exit 1
fi

echo
echo "🎉 Local development environment is ready!"
echo
echo "📋 Services available:"
echo "   • MongoDB: mongodb://localhost:27017"
echo "   • MongoDB Admin: http://localhost:8081 (admin/password)"
echo
echo "🔧 To use in your application:"
echo "   export MONGODB_URI='mongodb://localhost:27017'"
echo
echo "🛑 To stop the services:"
echo "   docker-compose down"
echo
echo "📊 To view logs:"
echo "   docker-compose logs -f"