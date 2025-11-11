# Getting Started with RAG Quant Trading Agent

## Welcome! 👋

This guide will help you get started with the RAG Quant Trading Agent project. Follow these steps to set up and run the system.

## Prerequisites Checklist

Before you begin, make sure you have:

- [ ] C++17 compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- [ ] CMake 3.20 or later
- [ ] Git
- [ ] Python 3.8 or later
- [ ] Alpha Vantage API key ([Get one here](https://www.alphavantage.co/support/#api-key))
- [ ] OpenAI API key ([Get one here](https://platform.openai.com/api-keys))

## Step 1: Clone the Repository

```bash
git clone <your-repo-url>
cd R.A.G.QuantTradingAgentProject
```

## Step 2: Run Setup Script

The setup script will:
- Create necessary directories
- Install vcpkg (if not already installed)
- Install dependencies
- Build the project

```bash
chmod +x setup.sh
./setup.sh
```

## Step 3: Configure API Keys

Set your API keys as environment variables:

```bash
export ALPHA_VANTAGE_API_KEY="your_alpha_vantage_key"
export OPENAI_API_KEY="your_openai_key"
```

Or create a `.env` file:

```bash
cp .env.example .env
# Edit .env with your API keys
```

## Step 4: Build the Project

If the setup script didn't build the project, do it manually:

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Step 5: Run the Server

```bash
cd build
./rag_agent_server
```

You should see:
```
[INFO] Starting RAG Quant Trading Agent Server
[INFO] RAG Agent initialized successfully
[INFO] Starting gRPC server on 0.0.0.0:50051
[INFO] Server listening on 0.0.0.0:50051
```

## Step 6: Ingest Market Data

In a new terminal:

```bash
python3 scripts/ingest_data.py --symbols AAPL MSFT GOOGL --days 30 --news
```

This will:
- Fetch historical stock data for the specified symbols
- Fetch news articles
- Store data in SQLite database

## Step 7: Build Vector Index

```bash
python3 scripts/build_vector_index.py --source all
```

This will:
- Retrieve documents from the database
- Generate embeddings
- Build FAISS index for semantic search

## Step 8: Test the Agent

Use the example client:

```bash
python3 scripts/client_example.py
```

Or use gRPC directly (after generating Python proto files):

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

## Common Issues and Solutions

### Issue: CMake can't find dependencies

**Solution**: Make sure vcpkg is installed and the toolchain file path is correct:

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Issue: API keys not found

**Solution**: Verify environment variables are set:

```bash
echo $ALPHA_VANTAGE_API_KEY
echo $OPENAI_API_KEY
```

### Issue: FAISS not found

**Solution**: Install FAISS manually. See [BUILD.md](docs/BUILD.md) for instructions.

### Issue: Port 50051 already in use

**Solution**: Change the server address in `src/main.cpp` or kill the process:

```bash
# Find process using port 50051
lsof -i :50051

# Kill the process
kill -9 <PID>
```

### Issue: Protobuf generation fails

**Solution**: Install protoc and gRPC plugin:

```bash
# Ubuntu/Debian
sudo apt-get install protobuf-compiler libprotobuf-dev

# macOS
brew install protobuf
```

## Next Steps

1. **Read the Documentation**: 
   - [README.md](README.md) - Main documentation
   - [ARCHITECTURE.md](docs/ARCHITECTURE.md) - System architecture
   - [API.md](docs/API.md) - API documentation

2. **Explore Examples**:
   - [client_example.py](scripts/client_example.py) - gRPC client example
   - [ingest_data.py](scripts/ingest_data.py) - Data ingestion example

3. **Customize**:
   - Modify RAG agent for your use case
   - Add more data sources
   - Extend the API

4. **Deploy**:
   - [DEPLOYMENT.md](docs/DEPLOYMENT.md) - Deployment guide
   - Use Docker for containerized deployment
   - Deploy to cloud (AWS, GCP, Azure)

## Getting Help

- **Documentation**: Check the `docs/` directory
- **Issues**: Open an issue on GitHub
- **Questions**: Check the troubleshooting section

## Example Queries

### Get Stock Summary

```python
request = StockSummaryRequest(symbol="AAPL", period="1m")
response = stub.GetStockSummary(request)
print(response.summary)
```

### Explain Volatility

```python
request = VolatilityRequest(symbol="AAPL", date="2024-01-15")
response = stub.ExplainVolatility(request)
print(response.explanation)
```

### Compare Sentiment

```python
request = SentimentCompareRequest(ticker1="AAPL", ticker2="MSFT", period="1m")
response = stub.CompareSentiment(request)
print(response.comparison)
```

### General Query

```python
request = QueryRequest(query="Why did NVIDIA stock surge?", symbols=["NVDA"])
response = stub.QueryRAG(request)
print(response.answer)
```

## Docker Quick Start

If you prefer Docker:

```bash
# Build image
docker build -t rag-quant-trading-agent .

# Run container
docker run -p 50051:50051 \
  -e ALPHA_VANTAGE_API_KEY=your_key \
  -e OPENAI_API_KEY=your_key \
  rag-quant-trading-agent
```

Or use Docker Compose:

```bash
# Create .env file with API keys
cp .env.example .env
# Edit .env

# Start services
docker-compose up -d
```

## What's Next?

1. **Ingest More Data**: Add more symbols and time periods
2. **Build Index**: Create vector index for semantic search
3. **Query Agent**: Test different queries and use cases
4. **Customize**: Modify for your specific needs
5. **Deploy**: Set up production deployment

Happy trading! 🚀

