#!/bin/bash

# Script to load environment variables from .env file
# Usage: source scripts/load_env.sh

if [ -f .env ]; then
    echo "Loading environment variables from .env file..."
    export $(cat .env | grep -v '^#' | xargs)
    echo "✅ Environment variables loaded"
    echo "  - ALPHA_VANTAGE_API_KEY: ${ALPHA_VANTAGE_API_KEY:0:10}..."
    echo "  - OPENAI_API_KEY: ${OPENAI_API_KEY:0:20}..."
else
    echo "⚠️  .env file not found"
    echo "Please create .env file with your API keys"
    exit 1
fi

