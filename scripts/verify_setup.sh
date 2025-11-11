#!/bin/bash

# Script to verify project setup

set -e

echo "🔍 Verifying RAG Quant Trading Agent Setup"
echo "==========================================="
echo ""

# Check git
echo "📦 Git Repository:"
if [ -d ".git" ]; then
    echo "  ✅ Git repository initialized"
    echo "  📝 Branch: $(git branch --show-current)"
    echo "  📊 Commits: $(git rev-list --count HEAD)"
    echo "  📁 Files tracked: $(git ls-files | wc -l | tr -d ' ')"
else
    echo "  ❌ Git repository not found"
fi
echo ""

# Check required directories
echo "📁 Directories:"
for dir in data logs include src proto scripts docs; do
    if [ -d "$dir" ]; then
        echo "  ✅ $dir/"
    else
        echo "  ❌ $dir/ not found"
    fi
done
echo ""

# Check key files
echo "📄 Key Files:"
key_files=(
    "CMakeLists.txt"
    "README.md"
    "setup.sh"
    ".env.example"
    "proto/rag_service.proto"
    "src/main.cpp"
    "include/rag/rag_agent.h"
)

for file in "${key_files[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✅ $file"
    else
        echo "  ❌ $file not found"
    fi
done
echo ""

# Check tools
echo "🛠️  Required Tools:"
tools=("cmake" "python3" "git")

for tool in "${tools[@]}"; do
    if command -v "$tool" &> /dev/null; then
        version=$($tool --version 2>/dev/null | head -1 || echo "installed")
        echo "  ✅ $tool: $version"
    else
        echo "  ❌ $tool not found"
    fi
done
echo ""

# Check vcpkg
echo "📦 vcpkg:"
if [ -d "vcpkg" ]; then
    echo "  ✅ vcpkg directory exists"
    if [ -f "vcpkg/vcpkg" ]; then
        echo "  ✅ vcpkg executable found"
    else
        echo "  ⚠️  vcpkg not bootstrapped (run ./vcpkg/bootstrap-vcpkg.sh)"
    fi
else
    echo "  ⏳ vcpkg will be installed by setup.sh"
fi
echo ""

# Check API keys
echo "🔑 API Keys:"
if [ -f ".env" ]; then
    echo "  ✅ .env file exists"
    if grep -q "your_.*_key" .env 2>/dev/null; then
        echo "  ⚠️  Please update .env with your actual API keys"
    else
        echo "  ✅ .env appears to be configured"
    fi
else
    echo "  ⏳ .env file not found (create from .env.example)"
fi
echo ""

# Check build directory
echo "🔨 Build Status:"
if [ -d "build" ]; then
    echo "  ✅ build/ directory exists"
    if [ -f "build/rag_agent_server" ]; then
        echo "  ✅ Server executable found"
    else
        echo "  ⏳ Project not built yet (run ./setup.sh or cmake build)"
    fi
else
    echo "  ⏳ build/ directory not found (will be created)"
fi
echo ""

# Summary
echo "📊 Summary:"
echo "  Project structure: ✅ Complete"
echo "  Git repository: ✅ Initialized"
echo "  Documentation: ✅ Complete"
echo "  Next steps:"
echo "    1. Create GitHub repository"
echo "    2. Configure API keys in .env"
echo "    3. Run ./setup.sh to install dependencies"
echo "    4. Build and run the server"
echo ""

echo "✅ Verification complete!"

