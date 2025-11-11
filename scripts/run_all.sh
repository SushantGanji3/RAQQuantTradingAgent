#!/bin/bash
# Quick start script to run the complete system

set -e

echo "🚀 RAG Quant Trading Agent - Quick Start"
echo "=========================================="
echo ""

# Check if server is already running
if lsof -Pi :50051 -sTCP:LISTEN -t >/dev/null ; then
    echo "⚠️  Server is already running on port 50051"
    echo "   To stop it, find the process: lsof -i :50051"
    echo ""
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Load environment variables
echo "📝 Loading environment variables..."
source scripts/load_env.sh

# Check if database exists, if not create directory
mkdir -p data

# Start server in background
echo "🖥️  Starting server..."
./build/rag_agent_server &
SERVER_PID=$!

# Wait for server to start
echo "⏳ Waiting for server to start..."
sleep 3

# Check if server started successfully
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Server failed to start"
    exit 1
fi

echo "✅ Server started (PID: $SERVER_PID)"
echo ""

# Ingest sample data
echo "📊 Ingesting sample data..."
python3 scripts/ingest_data.py --symbols AAPL MSFT --days 5 --news --max-articles 3

echo ""
echo "✅ Setup complete!"
echo ""
echo "Server is running in the background (PID: $SERVER_PID)"
echo "To stop the server: kill $SERVER_PID"
echo ""
echo "Test the server with:"
echo "  python3 scripts/test_client.py"
echo ""

# Keep script running (server runs in background)
wait $SERVER_PID

