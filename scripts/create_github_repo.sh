#!/bin/bash

# Script to help create and push to GitHub repository

set -e

echo "🚀 GitHub Repository Setup Helper"
echo "=================================="
echo ""

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "❌ Not a git repository. Please run 'git init' first."
    exit 1
fi

# Check if remote already exists
if git remote get-url origin >/dev/null 2>&1; then
    echo "✓ Remote 'origin' already exists:"
    git remote get-url origin
    echo ""
    read -p "Do you want to push to existing remote? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "📤 Pushing to GitHub..."
        git push -u origin main
        echo "✅ Done! Repository pushed to GitHub."
        exit 0
    else
        echo "Cancelled."
        exit 0
    fi
fi

# Check if GitHub CLI is installed
if command -v gh &> /dev/null; then
    echo "✓ GitHub CLI found"
    echo ""
    read -p "Do you want to create repository using GitHub CLI? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "📦 Creating GitHub repository..."
        read -p "Repository name (default: R.A.G.QuantTradingAgentProject): " repo_name
        repo_name=${repo_name:-R.A.G.QuantTradingAgentProject}
        
        read -p "Visibility (public/private, default: public): " visibility
        visibility=${visibility:-public}
        
        gh repo create "$repo_name" --$visibility --source=. --remote=origin --push
        echo "✅ Repository created and pushed to GitHub!"
        exit 0
    fi
fi

# Manual setup
echo "📝 Manual GitHub Repository Setup"
echo ""
echo "1. Go to https://github.com/new"
echo "2. Create a new repository named: R.A.G.QuantTradingAgentProject"
echo "3. DO NOT initialize with README, .gitignore, or license"
echo "4. Copy the repository URL"
echo ""
read -p "Paste the repository URL here: " repo_url

if [ -z "$repo_url" ]; then
    echo "❌ No URL provided. Exiting."
    exit 1
fi

echo ""
echo "📤 Adding remote and pushing..."
git remote add origin "$repo_url"
git push -u origin main

echo ""
echo "✅ Done! Repository pushed to GitHub."
echo "🔗 View your repository at: $repo_url"

