# Quick Start Guide

Get up and running with the RAG Quant Trading Agent in 5 minutes!

## Prerequisites

- C++17 compiler
- CMake 3.20+
- Python 3.8+
- API keys for Alpha Vantage and OpenAI

## Step 1: Clone and Setup

```bash
git clone <your-repo-url>
cd R.A.G.QuantTradingAgentProject
chmod +x setup.sh
./setup.sh
```

## Step 2: Set API Keys

```bash
export ALPHA_VANTAGE_API_KEY="your_key"
export OPENAI_API_KEY="your_key"
```

Or create a `.env` file:

```bash
cp .env.example .env
# Edit .env with your API keys
```

## Step 3: Build and Run

```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
./rag_agent_server
```

## Step 4: Test the Server

In another terminal:

```bash
python3 scripts/client_example.py
```

## Step 5: Ingest Data

```bash
python3 scripts/ingest_data.py --symbols AAPL MSFT --days 30 --news
```

## Step 6: Build Vector Index

```bash
python3 scripts/build_vector_index.py --source all
```

## Step 7: Query the Agent

Use the gRPC client to query the agent:

```python
import grpc
from proto import rag_service_pb2, rag_service_pb2_grpc

channel = grpc.insecure_channel('localhost:50051')
stub = rag_service_pb2_grpc.RAGAgentServiceStub(channel)

# Get stock summary
request = rag_service_pb2.StockSummaryRequest(symbol="AAPL", period="1m")
response = stub.GetStockSummary(request)
print(response.summary)
```

## Common Issues

### Issue: CMake can't find dependencies

**Solution**: Make sure vcpkg is installed and the toolchain file path is correct.

### Issue: API keys not found

**Solution**: Set environment variables or create a `.env` file.

### Issue: FAISS not found

**Solution**: Install FAISS manually (see BUILD.md for instructions).

### Issue: Port 50051 already in use

**Solution**: Change the server address in `src/main.cpp` or kill the process using the port.

## Next Steps

- Read the [README.md](README.md) for detailed documentation
- Check [BUILD.md](docs/BUILD.md) for build instructions
- See [API.md](docs/API.md) for API documentation
- Review [ARCHITECTURE.md](docs/ARCHITECTURE.md) for system architecture

## Getting Help

- Check the [troubleshooting section](README.md#troubleshooting)
- Open an issue on GitHub
- Read the documentation in the `docs/` directory

## Example Queries

### Get Stock Summary

```python
request = StockSummaryRequest(symbol="AAPL", period="1m")
response = stub.GetStockSummary(request)
```

### Explain Volatility

```python
request = VolatilityRequest(symbol="AAPL", date="2024-01-15")
response = stub.ExplainVolatility(request)
```

### Compare Sentiment

```python
request = SentimentCompareRequest(ticker1="AAPL", ticker2="MSFT", period="1m")
response = stub.CompareSentiment(request)
```

### General Query

```python
request = QueryRequest(query="Why did NVIDIA stock surge?", symbols=["NVDA"])
response = stub.QueryRAG(request)
```

## Docker Quick Start

```bash
docker build -t rag-quant-trading-agent .
docker run -p 50051:50051 \
  -e ALPHA_VANTAGE_API_KEY=your_key \
  -e OPENAI_API_KEY=your_key \
  rag-quant-trading-agent
```

## What's Next?

1. **Customize**: Modify the RAG agent for your specific use case
2. **Extend**: Add more data sources or LLM models
3. **Scale**: Deploy to production with Docker or Kubernetes
4. **Monitor**: Set up logging and monitoring
5. **Optimize**: Tune performance for your workload

Happy trading! 🚀

