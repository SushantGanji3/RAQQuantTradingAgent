#!/bin/bash

# Setup script for RAG Quant Trading Agent

set -e

echo "🚀 Setting up RAG Quant Trading Agent..."

# Check for required tools
command -v cmake >/dev/null 2>&1 || { echo "❌ CMake is required but not installed. Abort." >&2; exit 1; }
command -v git >/dev/null 2>&1 || { echo "❌ Git is required but not installed. Abort." >&2; exit 1; }

# Create directories
echo "📁 Creating directories..."
mkdir -p data
mkdir -p logs
mkdir -p build

# Check for vcpkg
if [ ! -d "vcpkg" ]; then
    echo "📦 Installing vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    cd ..
else
    echo "✓ vcpkg already installed"
fi

# Install dependencies
echo "📦 Installing dependencies..."
./vcpkg/vcpkg install curl nlohmann-json sqlite3 grpc protobuf abseil

# Check for FAISS
echo "⚠️  Note: FAISS needs to be installed manually"
echo "   See: https://github.com/facebookresearch/faiss"

# Check for API keys
if [ -z "$ALPHA_VANTAGE_API_KEY" ]; then
    echo "⚠️  Warning: ALPHA_VANTAGE_API_KEY not set"
fi

if [ -z "$OPENAI_API_KEY" ]; then
    echo "⚠️  Warning: OPENAI_API_KEY not set"
fi

# Build project
echo "🔨 Building project..."
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

echo "✅ Setup complete!"
echo ""
echo "To run the server:"
echo "  export ALPHA_VANTAGE_API_KEY=your_key"
echo "  export OPENAI_API_KEY=your_key"
echo "  ./build/rag_agent_server"

