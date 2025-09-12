#!/usr/bin/env python3
import requests
import json

BASE_URL = "http://localhost:8080"

def test_health():
    """Test the health check endpoint"""
    try:
        response = requests.get(f"{BASE_URL}/api/health")
        print(f"Health check: {response.status_code}")
        print(f"Response: {response.json()}")
        return response.status_code == 200
    except Exception as e:
        print(f"Health check failed: {e}")
        return False

def test_get_recipes():
    """Test getting all recipes"""
    try:
        response = requests.get(f"{BASE_URL}/api/recipes")
        print(f"Get recipes: {response.status_code}")
        if response.status_code == 200:
            data = response.json()
            print(f"Recipes count: {len(data.get('data', {}).get('recipes', []))}")
        return response.status_code == 200
    except Exception as e:
        print(f"Get recipes failed: {e}")
        return False

def test_add_recipe():
    """Test adding a new recipe"""
    recipe_data = {
        "title": "Test Pasta",
        "ingredients": "Pasta, Tomato Sauce, Cheese",
        "instructions": "Cook pasta, add sauce, sprinkle cheese",
        "servingSize": "4 servings",
        "cookTime": "20 minutes",
        "category": "Italian",
        "type": "Main Course"
    }

    try:
        response = requests.post(
            f"{BASE_URL}/api/recipes",
            json=recipe_data,
            headers={"Content-Type": "application/json"}
        )
        print(f"Add recipe: {response.status_code}")
        if response.status_code == 200:
            print(f"Response: {response.json()}")
        return response.status_code == 200
    except Exception as e:
        print(f"Add recipe failed: {e}")
        return False

def test_search_recipes():
    """Test searching recipes"""
    try:
        response = requests.get(f"{BASE_URL}/api/recipes/search?q=Pasta")
        print(f"Search recipes: {response.status_code}")
        if response.status_code == 200:
            data = response.json()
            print(f"Search results: {len(data.get('data', {}).get('recipes', []))}")
        return response.status_code == 200
    except Exception as e:
        print(f"Search recipes failed: {e}")
        return False

if __name__ == "__main__":
    print("Testing RecipeForADisaster Web API...")
    print("=" * 40)

    # Run tests
    tests = [
        ("Health Check", test_health),
        ("Get Recipes", test_get_recipes),
        ("Add Recipe", test_add_recipe),
        ("Search Recipes", test_search_recipes),
    ]

    passed = 0
    for name, test_func in tests:
        print(f"\nTesting {name}:")
        if test_func():
            passed += 1
            print("✓ PASSED")
        else:
            print("✗ FAILED")

    print(f"\nResults: {passed}/{len(tests)} tests passed")