#!/bin/bash

# RecipeForADisaster Production Deployment Script
# This script sets up and deploys the application in production mode

set -e

echo "🚀 Starting RecipeForADisaster Production Deployment"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    print_error "Docker is not installed. Please install Docker first."
    exit 1
fi

# Check if Docker Compose is installed
if ! command -v docker-compose &> /dev/null && ! docker compose version &> /dev/null; then
    print_error "Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

# Create necessary directories
print_status "Creating production directories..."
mkdir -p data logs/nginx nginx/ssl

# Set proper permissions
chmod 755 data logs/nginx

# Check if .env file exists
if [ ! -f ".env" ]; then
    print_warning ".env file not found. Copying from .env.example..."
    cp .env.example .env
    print_warning "Please edit .env file with your production values before continuing!"
    print_warning "Especially update JWT_SECRET, database paths, and Azure OpenAI credentials."
    exit 1
fi

# Validate critical environment variables
print_status "Validating environment configuration..."

if ! grep -q "JWT_SECRET=d3a94d963d2579b68a4addc3f6da661dd7fc9545efe5c72f42892d67c4500488" .env; then
    print_warning "JWT_SECRET appears to be changed from default - good!"
else
    print_warning "JWT_SECRET is still using the default value. Please change it for security!"
fi

# Build the application
print_status "Building Docker images..."
if command -v docker-compose &> /dev/null; then
    docker-compose build --no-cache
else
    docker compose build --no-cache
fi

# Start the services
print_status "Starting production services..."
if command -v docker-compose &> /dev/null; then
    docker-compose up -d
else
    docker compose up -d
fi

# Wait for services to be healthy
print_status "Waiting for services to start..."
sleep 10

# Check if the application is running
print_status "Checking application health..."
if curl -f http://localhost:8080/api/health &> /dev/null; then
    print_success "Application is running successfully!"
    print_success "🌐 API available at: http://localhost:8080/api"
    print_success "📊 Health check: http://localhost:8080/api/health"
else
    print_error "Application health check failed. Check logs with: docker-compose logs"
    exit 1
fi

# Display service status
print_status "Service Status:"
if command -v docker-compose &> /dev/null; then
    docker-compose ps
else
    docker compose ps
fi

print_success "🎉 Production deployment completed successfully!"
echo ""
echo "Useful commands:"
echo "  View logs: docker-compose logs -f"
echo "  Stop services: docker-compose down"
echo "  Restart services: docker-compose restart"
echo "  Update deployment: docker-compose up -d --build"
echo ""
print_warning "Remember to:"
print_warning "  1. Configure SSL certificates in nginx/ssl/ for HTTPS"
print_warning "  2. Update Azure OpenAI credentials in .env for AI features"
print_warning "  3. Set up proper firewall rules"
print_warning "  4. Configure backup strategy for data/ directory"
print_warning "  5. Monitor logs in logs/ directory"