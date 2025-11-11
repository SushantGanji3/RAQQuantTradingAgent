#!/bin/bash

# macOS dependency installation script

set -e

echo "🍎 Installing Dependencies for macOS"
echo "===================================="
echo ""

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "⚠️  Homebrew not found. Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "✅ Homebrew found: $(brew --version | head -1)"
fi

echo ""
echo "📦 Installing system dependencies..."

# Install CMake
if ! command -v cmake &> /dev/null; then
    echo "Installing CMake..."
    brew install cmake
else
    echo "✅ CMake already installed: $(cmake --version | head -1)"
fi

# Install other dependencies
echo "Installing curl, sqlite3, pkg-config..."
brew install curl sqlite3 pkg-config

echo ""
echo "📦 Setting up vcpkg..."

# Check if vcpkg exists
if [ ! -d "vcpkg" ]; then
    echo "Cloning vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    cd ..
else
    echo "✅ vcpkg directory exists"
    if [ ! -f "vcpkg/vcpkg" ]; then
        echo "Bootstrapping vcpkg..."
        cd vcpkg
        ./bootstrap-vcpkg.sh
        cd ..
    fi
fi

echo ""
echo "📦 Installing vcpkg packages..."
./vcpkg/vcpkg install curl nlohmann-json sqlite3 grpc protobuf abseil

echo ""
echo "🐍 Installing Python dependencies..."
pip3 install grpcio grpcio-tools protobuf

echo ""
echo "⚠️  FAISS Installation:"
echo "FAISS needs to be installed manually. Options:"
echo "1. Using Conda: conda install -c pytorch faiss-cpu"
echo "2. Build from source: See INSTALL_DEPENDENCIES.md"
echo ""

echo "✅ Dependencies installed (except FAISS)"
echo ""
echo "Next steps:"
echo "1. Install FAISS (see INSTALL_DEPENDENCIES.md)"
echo "2. Configure API keys in .env"
echo "3. Build the project:"
echo "   mkdir -p build"
echo "   cd build"
echo "   cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake"
echo "   cmake --build . --config Release"

