#!/bin/bash

# Vault development setup script for RecipeForADisaster

export VAULT_ADDR=http://127.0.0.1:8200
export VAULT_TOKEN=root

echo "Starting Vault in development mode..."
vault server -dev -dev-root-token-id="root" -dev-listen-address="127.0.0.1:8200" &
VAULT_PID=$!

echo "Waiting for Vault to start..."
sleep 3

echo "Setting up AI service credentials in Vault..."

# Create secrets for Azure OpenAI
vault kv put secret/azure-openai \
    api_key="your-azure-openai-api-key-here" \
    endpoint="https://your-resource.openai.azure.com/" \
    deployment_name="gpt-4" \
    api_version="2023-12-01-preview"

# Create secrets for other AI services (placeholders)
vault kv put secret/ai-services \
    openai_api_key="your-openai-api-key-here" \
    anthropic_api_key="your-anthropic-api-key-here"

# Create a policy for the application
vault policy write recipe-app-policy - <<EOF
path "secret/data/azure-openai" {
  capabilities = ["read"]
}

path "secret/data/ai-services" {
  capabilities = ["read"]
}
EOF

echo "Vault setup complete!"
echo "Root Token: root"
echo "VAULT_ADDR: http://127.0.0.1:8200"
echo ""
echo "To stop Vault, run: kill $VAULT_PID"
echo "To check secrets: vault kv get secret/azure-openai"

# Keep the script running to maintain Vault
wait $VAULT_PID