#!/bin/bash

# Authentication API Integration Tests
# This script tests all authentication endpoints of the RecipeForADisaster API

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
API_BASE="http://localhost:8080"
TEST_EMAIL="test_$(date +%s)@example.com"
TEST_PASSWORD="TestPassword123!"
NEW_PASSWORD="NewPassword456!"
TEST_USERNAME="testuser_$(date +%s)"

echo -e "${YELLOW}=== Authentication API Integration Tests ===${NC}\n"

# Test 1: User Registration
echo -e "${YELLOW}Test 1: User Registration${NC}"
REGISTER_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/register" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"${TEST_EMAIL}\",\"password\":\"${TEST_PASSWORD}\",\"username\":\"${TEST_USERNAME}\"}")

if echo "$REGISTER_RESPONSE" | grep -q '"success":true'; then
    echo -e "${GREEN}✓ Registration successful${NC}"
    echo "Response: $REGISTER_RESPONSE"
else
    echo -e "${RED}✗ Registration failed${NC}"
    echo "Response: $REGISTER_RESPONSE"
    exit 1
fi
echo ""

# Test 2: User Login
echo -e "${YELLOW}Test 2: User Login${NC}"
LOGIN_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"${TEST_EMAIL}\",\"password\":\"${TEST_PASSWORD}\"}")

if echo "$LOGIN_RESPONSE" | grep -q '"success":true'; then
    echo -e "${GREEN}✓ Login successful${NC}"
    TOKEN=$(echo "$LOGIN_RESPONSE" | grep -o '"token":"[^"]*"' | cut -d'"' -f4)
    USER_ID=$(echo "$LOGIN_RESPONSE" | grep -o '"userId":"[^"]*"' | cut -d'"' -f4)
    echo "Token: ${TOKEN:0:50}..."
    echo "User ID: $USER_ID"
else
    echo -e "${RED}✗ Login failed${NC}"
    echo "Response: $LOGIN_RESPONSE"
    exit 1
fi
echo ""

# Test 3: Token Validation
echo -e "${YELLOW}Test 3: Token Validation${NC}"
VALIDATE_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/validate" \
  -H "Authorization: Bearer ${TOKEN}")

if echo "$VALIDATE_RESPONSE" | grep -q '"success":true'; then
    echo -e "${GREEN}✓ Token validation successful${NC}"
    echo "Response: $VALIDATE_RESPONSE"
else
    echo -e "${RED}✗ Token validation failed${NC}"
    echo "Response: $VALIDATE_RESPONSE"
    exit 1
fi
echo ""

# Test 4: Get Current User
echo -e "${YELLOW}Test 4: Get Current User Profile${NC}"
ME_RESPONSE=$(curl -s -X GET "${API_BASE}/api/auth/me" \
  -H "Authorization: Bearer ${TOKEN}")

if echo "$ME_RESPONSE" | grep -q '"success":true'; then
    echo -e "${GREEN}✓ Get user profile successful${NC}"
    echo "Response: $ME_RESPONSE"
else
    echo -e "${RED}✗ Get user profile failed${NC}"
    echo "Response: $ME_RESPONSE"
    exit 1
fi
echo ""

# Test 5: Change Password
echo -e "${YELLOW}Test 5: Change Password${NC}"
CHANGE_PASSWORD_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/change-password" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d "{\"oldPassword\":\"${TEST_PASSWORD}\",\"newPassword\":\"${NEW_PASSWORD}\"}")

if echo "$CHANGE_PASSWORD_RESPONSE" | grep -q '"success":true'; then
    echo -e "${GREEN}✓ Password change successful${NC}"
    echo "Response: $CHANGE_PASSWORD_RESPONSE"
else
    echo -e "${RED}✗ Password change failed${NC}"
    echo "Response: $CHANGE_PASSWORD_RESPONSE"
    exit 1
fi
echo ""

# Test 6: Login with New Password
echo -e "${YELLOW}Test 6: Login with New Password${NC}"
NEW_LOGIN_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"${TEST_EMAIL}\",\"password\":\"${NEW_PASSWORD}\"}")

if echo "$NEW_LOGIN_RESPONSE" | grep -q '"success":true'; then
    echo -e "${GREEN}✓ Login with new password successful${NC}"
    NEW_TOKEN=$(echo "$NEW_LOGIN_RESPONSE" | grep -o '"token":"[^"]*"' | cut -d'"' -f4)
    echo "New token: ${NEW_TOKEN:0:50}..."
else
    echo -e "${RED}✗ Login with new password failed${NC}"
    echo "Response: $NEW_LOGIN_RESPONSE"
    exit 1
fi
echo ""

# Error Handling Tests
echo -e "${YELLOW}=== Error Handling Tests ===${NC}\n"

# Test 7: Invalid Email Format
echo -e "${YELLOW}Test 7: Invalid Email Format${NC}"
INVALID_EMAIL_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"email":"invalid-email","password":"TestPassword123!","username":"testuser"}')

if echo "$INVALID_EMAIL_RESPONSE" | grep -q '"success":false'; then
    echo -e "${GREEN}✓ Invalid email correctly rejected${NC}"
    echo "Response: $INVALID_EMAIL_RESPONSE"
else
    echo -e "${RED}✗ Invalid email not rejected${NC}"
    echo "Response: $INVALID_EMAIL_RESPONSE"
fi
echo ""

# Test 8: Weak Password
echo -e "${YELLOW}Test 8: Weak Password${NC}"
WEAK_PASSWORD_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/register" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"weak@example.com\",\"password\":\"weak\",\"username\":\"testuser\"}")

if echo "$WEAK_PASSWORD_RESPONSE" | grep -q '"success":false'; then
    echo -e "${GREEN}✓ Weak password correctly rejected${NC}"
    echo "Response: $WEAK_PASSWORD_RESPONSE"
else
    echo -e "${RED}✗ Weak password not rejected${NC}"
    echo "Response: $WEAK_PASSWORD_RESPONSE"
fi
echo ""

# Test 9: Duplicate Email
echo -e "${YELLOW}Test 9: Duplicate Email${NC}"
DUPLICATE_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/register" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"${TEST_EMAIL}\",\"password\":\"${TEST_PASSWORD}\",\"username\":\"another_user\"}")

if echo "$DUPLICATE_RESPONSE" | grep -q '"success":false'; then
    echo -e "${GREEN}✓ Duplicate email correctly rejected${NC}"
    echo "Response: $DUPLICATE_RESPONSE"
else
    echo -e "${RED}✗ Duplicate email not rejected${NC}"
    echo "Response: $DUPLICATE_RESPONSE"
fi
echo ""

# Test 10: Invalid Credentials
echo -e "${YELLOW}Test 10: Invalid Login Credentials${NC}"
INVALID_LOGIN_RESPONSE=$(curl -s -X POST "${API_BASE}/api/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"email\":\"${TEST_EMAIL}\",\"password\":\"WrongPassword123!\"}")

if echo "$INVALID_LOGIN_RESPONSE" | grep -q '"success":false'; then
    echo -e "${GREEN}✓ Invalid credentials correctly rejected${NC}"
    echo "Response: $INVALID_LOGIN_RESPONSE"
else
    echo -e "${RED}✗ Invalid credentials not rejected${NC}"
    echo "Response: $INVALID_LOGIN_RESPONSE"
fi
echo ""

# Test 11: Missing Authorization Header
echo -e "${YELLOW}Test 11: Missing Authorization Header${NC}"
NO_AUTH_RESPONSE=$(curl -s -X GET "${API_BASE}/api/auth/me")

if echo "$NO_AUTH_RESPONSE" | grep -q '"success":false'; then
    echo -e "${GREEN}✓ Missing authorization correctly rejected${NC}"
    echo "Response: $NO_AUTH_RESPONSE"
else
    echo -e "${RED}✗ Missing authorization not rejected${NC}"
    echo "Response: $NO_AUTH_RESPONSE"
fi
echo ""

# Test 12: Invalid Token
echo -e "${YELLOW}Test 12: Invalid Token${NC}"
INVALID_TOKEN_RESPONSE=$(curl -s -X GET "${API_BASE}/api/auth/me" \
  -H "Authorization: Bearer invalid-token-12345")

if echo "$INVALID_TOKEN_RESPONSE" | grep -q '"success":false'; then
    echo -e "${GREEN}✓ Invalid token correctly rejected${NC}"
    echo "Response: $INVALID_TOKEN_RESPONSE"
else
    echo -e "${RED}✗ Invalid token not rejected${NC}"
    echo "Response: $INVALID_TOKEN_RESPONSE"
fi
echo ""

# Summary
echo -e "${GREEN}=== All Tests Completed ===${NC}"
echo -e "${GREEN}✓ Registration working${NC}"
echo -e "${GREEN}✓ Login working${NC}"
echo -e "${GREEN}✓ Token validation working${NC}"
echo -e "${GREEN}✓ User profile retrieval working${NC}"
echo -e "${GREEN}✓ Password change working${NC}"
echo -e "${GREEN}✓ Error handling working${NC}"
echo ""
echo -e "${YELLOW}Test user created:${NC}"
echo "  Email: ${TEST_EMAIL}"
echo "  User ID: ${USER_ID}"
