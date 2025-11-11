#!/bin/bash

# Script to run the RAG agent server with environment variables loaded
# Usage: ./scripts/run_server.sh

set -e

# Get the project directory
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_DIR"

# Load environment variables
if [ -f .env ]; then
    echo "Loading environment variables from .env file..."
    export $(cat .env | grep -v '^#' | xargs)
else
    echo "⚠️  .env file not found"
    echo "Please create .env file with your API keys"
    exit 1
fi

# Check if API keys are set
if [ -z "$ALPHA_VANTAGE_API_KEY" ] || [ -z "$OPENAI_API_KEY" ]; then
    echo "❌ API keys not set in .env file"
    echo "Please add ALPHA_VANTAGE_API_KEY and OPENAI_API_KEY to .env"
    exit 1
fi

# Check if server executable exists
if [ ! -f "build/rag_agent_server" ]; then
    echo "❌ Server executable not found. Please build the project first:"
    echo "   cd build"
    echo "   cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "   cmake --build . --config Release"
    exit 1
fi

echo "✅ Starting RAG Quant Trading Agent Server..."
echo "   Server address: ${SERVER_ADDRESS:-0.0.0.0:50051}"
echo ""

# Run the server
cd build
./rag_agent_server

