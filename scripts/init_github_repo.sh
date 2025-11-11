#!/bin/bash

# Script to initialize GitHub repository

set -e

echo "🚀 Initializing GitHub repository for RAG Quant Trading Agent..."

# Check if git is installed
command -v git >/dev/null 2>&1 || { echo "❌ Git is required but not installed. Abort." >&2; exit 1; }

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "📦 Initializing git repository..."
    git init
fi

# Add all files
echo "📝 Adding files to git..."
git add .

# Create initial commit
echo "💾 Creating initial commit..."
git commit -m "Initial commit: RAG Quant Trading Agent

- C++ data ingestion layer with Alpha Vantage API
- SQLite database for market data storage
- FAISS vector index for semantic search
- OpenAI embeddings integration
- RAG agent with GPT-4 integration
- gRPC API gateway with Protocol Buffers
- Python bindings and scripts
- Docker support
- Comprehensive documentation"

echo "✅ Git repository initialized!"
echo ""
echo "Next steps:"
echo "1. Create a new repository on GitHub"
echo "2. Add remote: git remote add origin <your-repo-url>"
echo "3. Push: git push -u origin main"
echo ""
echo "Or use GitHub CLI:"
echo "  gh repo create R.A.G.QuantTradingAgentProject --public --source=. --remote=origin --push"

