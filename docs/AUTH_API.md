# Authentication API Documentation

## Overview

The RecipeForADisaster Authentication API provides secure user authentication and authorization using JWT (JSON Web Tokens). All authentication endpoints are prefixed with `/api/auth/`.

## Base URL

```
http://localhost:8080
```

## Authentication

Protected endpoints require a JWT token in the Authorization header:

```
Authorization: Bearer <your-jwt-token>
```

## Environment Configuration

The following environment variables can be configured:

| Variable | Description | Default | Required |
|----------|-------------|---------|----------|
| `JWT_SECRET` | Secret key for JWT signing | (warning shown) | Yes (production) |
| `JWT_ISSUER` | Token issuer identifier | "RecipeForADisaster" | No |
| `JWT_AUDIENCE` | Token audience identifier | "RecipeForADisaster-API" | No |
| `JWT_EXPIRATION_SECONDS` | Token lifetime in seconds | 3600 (1 hour) | No |

## Endpoints

### 1. User Registration

Register a new user account.

**Endpoint:** `POST /api/auth/register`

**Request Body:**
```json
{
  "email": "user@example.com",
  "password": "SecurePassword123!",
  "username": "johndoe"
}
```

**Validation Rules:**
- Email must be valid format
- Password must be at least 8 characters with uppercase, lowercase, and numbers
- Email must be unique

**Success Response (200):**
```json
{
  "success": true,
  "message": "User registered successfully",
  "data": {
    "userId": "550e8400-e29b-41d4-a716-446655440000",
    "email": "user@example.com"
  }
}
```

**Error Responses:**
- `400` - Invalid email format
- `400` - Password too weak
- `409` - User with this email already exists

**Example:**
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"email":"user@example.com","password":"SecurePass123!","username":"johndoe"}'
```

---

### 2. User Login

Authenticate a user and receive a JWT token.

**Endpoint:** `POST /api/auth/login`

**Request Body:**
```json
{
  "email": "user@example.com",
  "password": "SecurePassword123!"
}
```

**Success Response (200):**
```json
{
  "success": true,
  "message": "Login successful",
  "data": {
    "userId": "550e8400-e29b-41d4-a716-446655440000",
    "email": "user@example.com",
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
  }
}
```

**Error Responses:**
- `400` - Missing email or password
- `401` - Invalid email or password
- `403` - User account is not active

**Example:**
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"user@example.com","password":"SecurePass123!"}'
```

---

### 3. Validate Token

Validate a JWT token and retrieve user information.

**Endpoint:** `POST /api/auth/validate`

**Headers:**
```
Authorization: Bearer <your-jwt-token>
```

**Success Response (200):**
```json
{
  "success": true,
  "message": "Token validated successfully",
  "data": {
    "userId": "550e8400-e29b-41d4-a716-446655440000",
    "email": "user@example.com"
  }
}
```

**Error Responses:**
- `401` - No token provided
- `401` - Invalid or expired token

**Example:**
```bash
curl -X POST http://localhost:8080/api/auth/validate \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

---

### 4. Get Current User

Retrieve the authenticated user's profile information.

**Endpoint:** `GET /api/auth/me`

**Headers:**
```
Authorization: Bearer <your-jwt-token>
```

**Success Response (200):**
```json
{
  "success": true,
  "data": {
    "id": "550e8400-e29b-41d4-a716-446655440000",
    "email": "user@example.com",
    "isActive": true
  }
}
```

**Error Responses:**
- `401` - No token provided
- `401` - Invalid or expired token
- `404` - User not found

**Example:**
```bash
curl -X GET http://localhost:8080/api/auth/me \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

---

### 5. Change Password

Change the authenticated user's password.

**Endpoint:** `POST /api/auth/change-password`

**Headers:**
```
Authorization: Bearer <your-jwt-token>
```

**Request Body:**
```json
{
  "oldPassword": "OldPassword123!",
  "newPassword": "NewSecurePassword456!"
}
```

**Validation Rules:**
- Old password must match current password
- New password must be at least 8 characters with uppercase, lowercase, and numbers
- New password must be different from old password

**Success Response (200):**
```json
{
  "success": true,
  "message": "Password changed successfully"
}
```

**Error Responses:**
- `400` - Old password and new password are required
- `400` - Invalid old password
- `400` - Failed to change password
- `401` - No token provided
- `401` - Invalid or expired token

**Example:**
```bash
curl -X POST http://localhost:8080/api/auth/change-password \
  -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..." \
  -H "Content-Type: application/json" \
  -d '{"oldPassword":"OldPass123!","newPassword":"NewSecurePass456!"}'
```

---

## JWT Token Structure

The JWT token contains the following claims:

```json
{
  "sub": "550e8400-e29b-41d4-a716-446655440000",
  "email": "user@example.com",
  "iss": "RecipeForADisaster",
  "aud": "RecipeForADisaster-API",
  "iat": 1234567890,
  "exp": 1234571490,
  "nbf": 1234567890
}
```

- `sub` - User ID (subject)
- `email` - User email address
- `iss` - Issuer (configured via JWT_ISSUER)
- `aud` - Audience (configured via JWT_AUDIENCE)
- `iat` - Issued at timestamp
- `exp` - Expiration timestamp
- `nbf` - Not before timestamp

---

## Error Response Format

All error responses follow this format:

```json
{
  "success": false,
  "error": "Error message description"
}
```

---

## Common HTTP Status Codes

| Code | Meaning | Description |
|------|---------|-------------|
| 200 | OK | Request successful |
| 400 | Bad Request | Invalid request data or validation failure |
| 401 | Unauthorized | Missing, invalid, or expired authentication token |
| 403 | Forbidden | User account is inactive or lacks permission |
| 404 | Not Found | Requested resource not found |
| 409 | Conflict | Resource already exists (e.g., duplicate email) |
| 500 | Internal Server Error | Unexpected server error |

---

## Security Considerations

1. **Password Requirements:**
   - Minimum 8 characters
   - At least one uppercase letter
   - At least one lowercase letter
   - At least one number
   - Special characters recommended

2. **Token Security:**
   - Tokens expire after configured duration (default: 1 hour)
   - Tokens are signed with JWT_SECRET
   - Store tokens securely (never in local storage for production)
   - Use HTTPS in production

3. **Password Storage:**
   - Passwords are hashed using SHA-256
   - Original passwords are never stored

4. **Rate Limiting:**
   - Consider implementing rate limiting for login attempts
   - Monitor for brute force attacks

---

## Testing

Run the comprehensive test suite:

```bash
# Start the server with JWT configuration
cd build
JWT_SECRET="test-secret-key-12345" ./web_server &

# Run integration tests
cd ../tests
./test_auth_api.sh
```

---

## Example Workflow

### Complete Authentication Flow

```bash
# 1. Register a new user
REGISTER_RESPONSE=$(curl -s -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"email":"user@example.com","password":"SecurePass123!","username":"johndoe"}')

# 2. Login to get JWT token
LOGIN_RESPONSE=$(curl -s -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"user@example.com","password":"SecurePass123!"}')

# Extract token from response
TOKEN=$(echo $LOGIN_RESPONSE | jq -r '.data.token')

# 3. Validate token
curl -X POST http://localhost:8080/api/auth/validate \
  -H "Authorization: Bearer $TOKEN"

# 4. Get user profile
curl -X GET http://localhost:8080/api/auth/me \
  -H "Authorization: Bearer $TOKEN"

# 5. Change password
curl -X POST http://localhost:8080/api/auth/change-password \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"oldPassword":"SecurePass123!","newPassword":"NewSecurePass456!"}'

# 6. Login with new password
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"user@example.com","password":"NewSecurePass456!"}'
```

---

## Integration with Recipe API

Protected recipe endpoints can use the same JWT token:

```bash
# Get user's recipes (example of protected endpoint)
curl -X GET http://localhost:8080/api/recipes/my-recipes \
  -H "Authorization: Bearer $TOKEN"
```

---

## Troubleshooting

### "No token provided" Error
Ensure the Authorization header is present:
```
Authorization: Bearer <token>
```

### "Invalid or expired token" Error
- Token may have expired (default: 1 hour)
- Token may be malformed
- JWT_SECRET may have changed
- Solution: Login again to get a new token

### "User with this email already exists" Error
- Email is already registered
- Solution: Use a different email or login with existing account

### Server won't start - "Address already in use"
- Port 8080 is already in use
- Solution: Kill existing process or change port

---

## Implementation Details

### Technologies Used
- **Web Framework:** Crow (C++ micro web framework)
- **JWT Library:** jwt-cpp v0.7.0
- **Database:** SQLite3
- **Cryptography:** OpenSSL (SHA-256 for password hashing)

### Database Schema

Users table:
```sql
CREATE TABLE IF NOT EXISTS users (
    id TEXT PRIMARY KEY,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    is_active INTEGER DEFAULT 1,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP
)
```

---

## Future Enhancements

Potential improvements for the authentication system:

1. **Email Verification:** Send verification email after registration
2. **Password Reset:** Forgot password functionality with email
3. **Refresh Tokens:** Long-lived refresh tokens for token renewal
4. **Multi-Factor Authentication:** 2FA support
5. **OAuth Integration:** Social login (Google, GitHub, etc.)
6. **Role-Based Access Control:** User roles and permissions
7. **Session Management:** Active session tracking and revocation
8. **Account Lockout:** Prevent brute force attacks
9. **Audit Logging:** Track authentication events
10. **Password History:** Prevent password reuse
