#!/bin/bash

# Script to prepare for GitHub repository creation

set -e

echo "🚀 Preparing for GitHub Repository Creation"
echo "============================================"
echo ""

# Check if we're in a git repo
if [ ! -d ".git" ]; then
    echo "❌ Not a git repository. Please run 'git init' first."
    exit 1
fi

# Check if remote exists
if git remote get-url origin >/dev/null 2>&1; then
    echo "✅ Remote 'origin' already configured:"
    git remote get-url origin
    echo ""
    echo "To push to GitHub, run:"
    echo "  git push -u origin main"
    exit 0
fi

# Check GitHub CLI
if command -v gh &> /dev/null; then
    echo "✅ GitHub CLI found"
    echo ""
    echo "You can create the repository using:"
    echo "  gh repo create R.A.G.QuantTradingAgentProject --public --source=. --remote=origin --push"
    echo ""
    read -p "Do you want to create the repository now? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        gh repo create R.A.G.QuantTradingAgentProject --public --source=. --remote=origin --push
        echo "✅ Repository created and pushed to GitHub!"
        exit 0
    fi
fi

# Manual instructions
echo "📝 Manual GitHub Repository Setup"
echo ""
echo "To create the repository on GitHub:"
echo ""
echo "1. Go to: https://github.com/new"
echo "2. Repository name: R.A.G.QuantTradingAgentProject"
echo "3. Description: RAG-based quantitative trading agent with LLM integration"
echo "4. Visibility: Public (or Private)"
echo "5. DO NOT initialize with README, .gitignore, or license"
echo "6. Click 'Create repository'"
echo ""
echo "Then run these commands:"
echo ""
echo "  git remote add origin https://github.com/YOUR_USERNAME/R.A.G.QuantTradingAgentProject.git"
echo "  git push -u origin main"
echo ""
echo "Replace YOUR_USERNAME with your GitHub username."
echo ""

# Show current status
echo "📊 Current Git Status:"
echo "  Branch: $(git branch --show-current)"
echo "  Commits: $(git rev-list --count HEAD)"
echo "  Files: $(git ls-files | wc -l | tr -d ' ')"
echo ""

# Check if .env is in .gitignore
if grep -q "^\.env$" .gitignore 2>/dev/null; then
    echo "✅ .env is in .gitignore (API keys will not be committed)"
else
    echo "⚠️  .env is not in .gitignore - consider adding it"
fi

echo ""
echo "✅ Ready to push to GitHub!"

