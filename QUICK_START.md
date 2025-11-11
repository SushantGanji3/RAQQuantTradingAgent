# Quick Start Guide

## ✅ Completed Steps

### 1. ✅ Project Built Successfully
- C++ server compiled and ready
- All dependencies installed via vcpkg
- Server executable: `build/rag_agent_server`

### 2. ✅ Python Protobuf Files Generated
- Location: `python_client/`
- Files: `rag_service_pb2.py`, `rag_service_pb2_grpc.py`

### 3. ✅ Data Ingestion Script Created
- Script: `scripts/ingest_data.py`
- Fetches data from Alpha Vantage API
- Stores data in SQLite database
- Successfully tested with AAPL and MSFT

### 4. ✅ Test Client Created
- Script: `scripts/test_client.py`
- Tests all gRPC endpoints
- Ready to use once server is running

## 🚀 Next Steps to Run the System

### Step 1: Start the Server

In one terminal, start the gRPC server:

```bash
# Load environment variables and start server
source scripts/load_env.sh
./build/rag_agent_server
```

You should see:
```
[INFO] Starting RAG Quant Trading Agent Server
[INFO] Database tables created successfully
[INFO] FAISS index initialized (stub mode) with dimension: 1536
[INFO] RAG Agent initialized successfully
[INFO] Starting gRPC server on 0.0.0.0:50051
[INFO] Server listening on 0.0.0.0:50051
```

### Step 2: Ingest Data (in another terminal)

```bash
# Ingest data for stocks
python3 scripts/ingest_data.py --symbols AAPL MSFT GOOGL --days 30 --news --max-articles 10
```

This will:
- Fetch real-time quotes
- Fetch historical OHLCV data
- Fetch news articles
- Store everything in the database

### Step 3: Test the Server

```bash
# Test the gRPC endpoints
python3 scripts/test_client.py
```

This will test:
- GetStockSummary
- QueryRAG
- ExplainVolatility

## 📊 Current Data Status

- **OHLCV Data**: 12 records in database
- **News Articles**: 6 articles in database
- **Database**: `data/trading_data.db`

## 🔧 Available Scripts

- `scripts/ingest_data.py` - Ingest market data from Alpha Vantage
- `scripts/test_client.py` - Test gRPC server endpoints
- `scripts/load_env.sh` - Load environment variables
- `scripts/run_server.sh` - Run the server (wrapper script)

## 📝 Example Usage

### Ingest Data
```bash
python3 scripts/ingest_data.py --symbols AAPL MSFT --days 30 --news
```

### Test Server (requires server to be running)
```bash
python3 scripts/test_client.py
```

### Query via gRPC
```python
import grpc
from python_client import rag_service_pb2, rag_service_pb2_grpc

channel = grpc.insecure_channel('localhost:50051')
stub = rag_service_pb2_grpc.RAGAgentServiceStub(channel)

request = rag_service_pb2.StockSummaryRequest(symbol='AAPL', period='1m')
response = stub.GetStockSummary(request)
print(response.summary)
```

## ⚠️ Notes

1. **FAISS**: Currently running in stub mode (vector search limited). Install FAISS for full functionality.
2. **API Keys**: Must be set in `.env` file (already configured)
3. **Server**: Must be running before testing with client
4. **Database**: Tables are created automatically by the C++ server

## 🎯 Next Improvements

1. Install FAISS for full vector search capabilities
2. Build vector index from ingested news articles
3. Add more data sources (options, fundamentals)
4. Implement vectorization of stored documents
5. Test RAG queries with actual context retrieval

