---
name: Action Items [AI] - RecipeForADisaster Project
description: Track and manage action items for database implementation, AI services, and Vault integration
title: "[AI] Action Items Update"
labels: ["action-items", "documentation", "enhancement"]
assignees: []
body:
- type: markdown
  attributes:
    value: |
      # Action Items [AI] - RecipeForADisaster Project

      ## Database Implementation (MongoDB + Vault Integration)

      ### Remaining Action Items
      - [ ] **Update main.cpp**: Modify the main application to use Vault-enabled recipeManager constructor instead of environment variable constructor
      - [ ] **Implement fallback logic**: Ensure recipeManager gracefully falls back to environment variables when Vault is unavailable
      - [ ] **Test MongoDB credential retrieval**: Verify that MongoDB URI can be successfully retrieved from Vault path `database/mongodb`
      - [ ] **Update documentation**: Add Vault configuration requirements to README.md for MongoDB credentials
      - [ ] **Environment variable cleanup**: Remove MONGODB_URI from .env.example once Vault integration is fully tested

      ### Current Status
      - ✅ VaultService class implemented with credential retrieval
      - ✅ recipeManager updated with Vault constructor (`recipeManager(VaultService*, vaultPath)`)
      - ✅ CMake build system updated to include Vault dependencies
      - ✅ Build issues resolved (linking, dependencies)

      ## Azure AI Service (Azure OpenAI + Vault Integration)

      ### Remaining Action Items
      - [ ] **Update web_server.cpp**: Modify web server initialization to use Vault-enabled AIService constructor
      - [ ] **Implement fallback logic**: Ensure AIService gracefully falls back to environment variables when Vault is unavailable
      - [ ] **Test Azure OpenAI credential retrieval**: Verify that endpoint, API key, and deployment name can be retrieved from Vault path `ai/azure-openai`
      - [ ] **Update documentation**: Add Vault configuration requirements to README.md for Azure OpenAI credentials
      - [ ] **Environment variable cleanup**: Remove AZURE_OPENAI_KEY from .env.example once Vault integration is fully tested

      ### Current Status
      - ✅ VaultService class implemented with credential retrieval
      - ✅ AIService updated with Vault constructor (`AIService(VaultService*, vaultPath)`)
      - ✅ CMake build system updated to include Vault dependencies
      - ✅ Build issues resolved (linking, dependencies)

      ## Current Issues and Blockers

      ### Build System Issues (RESOLVED)
      - ✅ **Duplicate WriteCallback symbols**: Fixed by creating shared `common_utils.cpp` with single implementation
      - ✅ **Missing source files**: Updated CMakeLists.txt to include `common_utils.cpp` in all relevant targets
      - ✅ **Linker errors**: Resolved by adding proper dependencies and source files to build targets

      ### Runtime/Integration Issues
      - [ ] **Vault server availability**: Need actual Vault server for testing (currently using mock configurations)
      - [ ] **Network connectivity**: Vault service requires network access to Vault server
      - [ ] **Authentication tokens**: Need valid Vault tokens for production use
      - [ ] **SSL/TLS configuration**: Vault connections may require certificate configuration

      ### Testing Infrastructure Issues
      - [ ] **Mock Vault server**: Need test setup with mock Vault server for CI/CD
      - [ ] **Credential seeding**: Need mechanism to populate test credentials in Vault
      - [ ] **Environment isolation**: Separate Vault paths for development, staging, production

      ## Test Cases Required

      ### Vault Service Tests
      - [ ] **Connection test**: Verify VaultService can connect to Vault server with valid token
      - [ ] **Authentication failure**: Test behavior when invalid token provided
      - [ ] **Network failure**: Test behavior when Vault server unreachable
      - [ ] **Secret retrieval**: Test successful retrieval of various secret types (strings, JSON)
      - [ ] **Secret not found**: Test behavior when requested secret path/key doesn't exist

      ### Database Integration Tests
      - [ ] **Vault credential retrieval**: Test MongoDB connection using credentials from Vault
      - [ ] **Fallback to environment**: Test MongoDB connection using environment variables when Vault unavailable
      - [ ] **Connection pooling**: Verify connection reuse works with Vault credentials
      - [ ] **Credential rotation**: Test behavior when Vault credentials change during runtime

      ### AI Service Integration Tests
      - [ ] **Vault credential retrieval**: Test Azure OpenAI connection using credentials from Vault
      - [ ] **Fallback to environment**: Test Azure OpenAI connection using environment variables when Vault unavailable
      - [ ] **API rate limiting**: Verify credential changes don't break rate limiting
      - [ ] **Token refresh**: Test behavior when Azure OpenAI tokens need rotation

      ### End-to-End Integration Tests
      - [ ] **Web server startup**: Test web server initializes with Vault services
      - [ ] **Recipe generation**: Test AI-powered recipe generation with Vault credentials
      - [ ] **Database operations**: Test full CRUD operations with Vault-backed database
      - [ ] **Error scenarios**: Test graceful degradation when Vault services fail

      ### Security Tests
      - [ ] **Credential exposure**: Verify credentials never logged or exposed in error messages
      - [ ] **Memory cleanup**: Ensure credentials properly cleared from memory after use
      - [ ] **Access control**: Test that only authorized services can access specific credential paths
      - [ ] **Audit logging**: Verify credential access is properly logged for compliance

      ## Infrastructure Requirements

      ### Vault Server Setup
      - [ ] **Development Vault**: Local Vault server for development testing
      - [ ] **CI/CD Vault**: Vault instance for automated testing
      - [ ] **Production Vault**: Secure Vault server for production credentials
      - [ ] **Backup/DR**: Vault backup and disaster recovery procedures

      ### Configuration Management
      - [ ] **Vault policies**: Define access policies for different credential paths
      - [ ] **Token management**: Implement token rotation and renewal strategies
      - [ ] **Secret versioning**: Enable versioning for credential history and rollback
      - [ ] **Monitoring**: Set up monitoring and alerting for Vault health

      ## Documentation Updates Required

      ### README.md Updates
      - [ ] **Vault configuration section**: Add detailed Vault setup instructions
      - [ ] **Environment variables**: Update to reflect Vault-first approach with env fallbacks
      - [ ] **Security considerations**: Document Vault security benefits and requirements
      - [ ] **Deployment guide**: Add Vault configuration to deployment procedures

      ### API Documentation
      - [ ] **Configuration endpoints**: Document Vault configuration requirements
      - [ ] **Health checks**: Update health check endpoints to include Vault status
      - [ ] **Error responses**: Document Vault-related error responses

      ## Risk Assessment

      ### High Risk Items
      - **Credential rotation**: Need to ensure zero-downtime credential rotation
      - **Vault availability**: Single point of failure if Vault becomes unavailable
      - **Token management**: Secure token storage and rotation strategy required

      ### Medium Risk Items
      - **Build complexity**: Increased build dependencies and complexity
      - **Testing complexity**: More complex test setup with Vault mocking
      - **Operational complexity**: Additional operational burden for Vault management

      ### Low Risk Items
      - **Code changes**: Vault integration follows existing patterns
      - **Performance impact**: Minimal performance impact from Vault lookups
      - **Backward compatibility**: Environment variable fallbacks maintain compatibility

- type: checkboxes
  id: completed-items
  attributes:
    label: Completed Action Items
    description: Check off any action items that have been completed since this template was last used
    options: []
  validations:
    required: false

- type: textarea
  id: progress-notes
  attributes:
    label: Progress Notes
    description: Add any notes about progress made, blockers encountered, or changes to the action items
    placeholder: |
      ## Recent Progress
      - [Date] Description of work completed

      ## Current Blockers
      - Issue description and potential solutions

      ## Updated Priorities
      - Any changes to the action items or priorities
  validations:
    required: false

- type: dropdown
  id: priority
  attributes:
    label: Priority Level
    description: How critical is addressing these action items?
    options:
      - High - Blocks core functionality
      - Medium - Important but not blocking
      - Low - Nice to have improvements
  validations:
    required: true

- type: textarea
  id: additional-context
  attributes:
    label: Additional Context
    description: Any additional information, links, or context that would be helpful
    placeholder: |
      - Related issues/PRs:
      - Documentation links:
      - Test results:
      - Deployment notes:
  validations:
    required: false