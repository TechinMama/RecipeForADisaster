#!/bin/bash

# RecipeForADisaster Health Check Script
# Tests the production deployment and reports status

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

echo "🔍 RecipeForADisaster Health Check"
echo "=================================="

# Check if Docker services are running
print_status "Checking Docker services..."
if docker ps | grep -q recipeforadisaster_app; then
    print_success "Docker services are running"
else
    print_error "Docker services are not running"
    echo "Run: docker-compose up -d"
    exit 1
fi

# Wait a moment for services to be ready
sleep 2

# Check API health
print_status "Checking API health..."
if curl -s -f http://localhost:8080/api/health > /dev/null 2>&1; then
    print_success "API health check passed"
else
    print_error "API health check failed"
    echo "Check logs: docker-compose logs app"
    exit 1
fi

# Check authentication endpoints
print_status "Checking authentication endpoints..."
if curl -s -f http://localhost:8080/api/auth/me > /dev/null 2>&1; then
    print_success "Authentication endpoints accessible"
else
    print_warning "Authentication endpoints may require authentication"
fi

# Check recipes endpoints
print_status "Checking recipes endpoints..."
if curl -s -f http://localhost:8080/api/recipes > /dev/null 2>&1; then
    print_success "Recipes endpoints accessible"
else
    print_error "Recipes endpoints not accessible"
    exit 1
fi

# Check collections endpoints
print_status "Checking collections endpoints..."
if curl -s -f -w "%{http_code}" http://localhost:8080/api/collections 2>/dev/null | grep -q "401\|200"; then
    print_success "Collections endpoints accessible"
else
    print_error "Collections endpoints not accessible"
    exit 1
fi

# Check database files
print_status "Checking database files..."
if docker exec recipeforadisaster_app ls -la /app/data/ | grep -q "\.db$"; then
    print_success "Database files exist"
else
    print_warning "Database files not found (may be created on first use)"
fi

# Check logs
print_status "Checking for error logs..."
if docker-compose logs --tail=50 app 2>&1 | grep -i error > /dev/null; then
    print_warning "Errors found in logs - check with: docker-compose logs app"
else
    print_success "No recent errors in logs"
fi

echo ""
print_success "🎉 Health check completed successfully!"
echo ""
echo "Service URLs:"
echo "  API: http://localhost:8080/api"
echo "  Health: http://localhost:8080/api/health"
echo "  Recipes: http://localhost:8080/api/recipes"
echo ""
echo "Management commands:"
echo "  View logs: docker-compose logs -f"
echo "  Restart: docker-compose restart"
echo "  Stop: docker-compose down"