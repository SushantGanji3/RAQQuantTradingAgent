# RAG Quant Trading Agent

A Retrieval-Augmented Generation (RAG) agent that connects live and historical market data (stocks, options, fundamentals) with a large language model (LLM) to produce context-aware trading insights.

## 🎯 Project Overview

This system combines:
- **C++ Data Ingestion Layer**: Efficiently fetches and stores real-time and historical financial data
- **Vectorization & Retrieval**: Stores financial documents in a vector database for semantic search using FAISS
- **RAG Generation**: Uses LLMs (GPT-4, Gemini) to generate insights augmented by retrieved data
- **gRPC API Gateway**: Serves user requests through a structured agent interface

## 🏗️ Architecture

```
┌──────────────────────────────┐
│     User Query (Frontend)    │
└──────────────┬───────────────┘
               │ gRPC Request
               ▼
┌──────────────────────────────┐
│   C++ Agent API Gateway       │
└──────────────┬───────────────┘
               │
               ▼
┌────────────────────────────────────────────────┐
│   Data + RAG Pipeline                           │
│  ┌──────────────────────────────┐               │
│  │  C++ Ingestion + Preprocessing│               │
│  └──────────────┬───────────────┘               │
│                 ▼                               │
│     FAISS Index + SQLite Storage (C++)           │
│                 ▼                               │
│       LLM Query (via REST or gRPC)              │
└────────────────────────────────────────────────┘
               │
               ▼
┌──────────────────────────────┐
│   Final Report / Explanation  │
└──────────────────────────────┘
```

## 📋 Prerequisites

- **C++17** or later
- **CMake 3.20+**
- **vcpkg** (for dependency management)
- **Python 3.8+** (for scripts and optional Python bindings)
- **API Keys**:
  - Alpha Vantage API key (for market data)
  - OpenAI API key (for embeddings and LLM)

## 🚀 Quick Start

### Option 1: Quick Setup (Recommended)

```bash
# Clone repository
git clone <your-repo-url>
cd R.A.G.QuantTradingAgentProject

# Run setup script
chmod +x setup.sh
./setup.sh

# Set API keys
export ALPHA_VANTAGE_API_KEY="your_key"
export OPENAI_API_KEY="your_key"

# Build and run
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
./rag_agent_server
```

### Option 2: Manual Setup

### 1. Clone the Repository

```bash
git clone <your-repo-url>
cd R.A.G.QuantTradingAgentProject
```

### 2. Initialize GitHub Repository (First Time)

If you're setting up a new repository:

```bash
# Initialize git and create initial commit
./scripts/init_github_repo.sh

# Create repository on GitHub and push
gh repo create R.A.G.QuantTradingAgentProject --public --source=. --remote=origin --push
```

### 3. Install Dependencies

#### Using vcpkg:

```bash
# Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Install dependencies
./vcpkg install curl nlohmann-json sqlite3 grpc protobuf abseil

# Install FAISS (may require manual installation)
# See: https://github.com/facebookresearch/faiss
```

#### System Dependencies:

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libcurl4-openssl-dev \
    libsqlite3-dev pkg-config

# macOS
brew install cmake curl sqlite3 pkg-config
```

### 3. Set Environment Variables

```bash
export ALPHA_VANTAGE_API_KEY="your_alpha_vantage_key"
export OPENAI_API_KEY="your_openai_key"
```

### 4. Build the Project

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### 5. Run the Server

```bash
./rag_agent_server
```

The server will start on `0.0.0.0:50051` by default.

## 📦 Project Structure

```
R.A.G.QuantTradingAgentProject/
├── CMakeLists.txt          # CMake build configuration
├── vcpkg.json              # vcpkg dependencies
├── proto/                  # Protocol Buffer definitions
│   └── rag_service.proto
├── include/                # Header files
│   ├── data_ingestion/
│   ├── vectorization/
│   ├── rag/
│   ├── api/
│   └── utils/
├── src/                    # Source files
│   ├── data_ingestion/
│   ├── vectorization/
│   ├── rag/
│   ├── api/
│   ├── python_bindings/
│   └── utils/
├── scripts/                # Utility scripts
│   ├── ingest_data.py
│   ├── build_vector_index.py
│   └── client_example.py
├── data/                   # Data storage (created at runtime)
├── logs/                   # Log files (created at runtime)
└── README.md
```

## 🔧 Configuration

### Database Configuration

The SQLite database is stored at `data/trading_data.db` by default. You can change this in `src/main.cpp`.

### FAISS Index

The FAISS index is stored at `data/faiss_index.index` by default. The index is automatically loaded on startup if it exists.

### API Endpoints

The gRPC server exposes the following services:

- `GetStockSummary`: Get stock summary with context
- `ExplainVolatility`: Explain volatility spike
- `CompareSentiment`: Compare sentiment between two tickers
- `RecommendPair`: Recommend long/short pair
- `QueryRAG`: General RAG query

## 📊 Usage Examples

### Ingest Market Data

```bash
python3 scripts/ingest_data.py --symbols AAPL MSFT GOOGL --days 30 --news
```

### Build Vector Index

```bash
python3 scripts/build_vector_index.py --source all
```

### Query via gRPC Client

```python
import grpc
from proto import rag_service_pb2, rag_service_pb2_grpc

channel = grpc.insecure_channel('localhost:50051')
stub = rag_service_pb2_grpc.RAGAgentServiceStub(channel)

# Get stock summary
request = rag_service_pb2.StockSummaryRequest(symbol="AAPL", period="1m")
response = stub.GetStockSummary(request)
print(response.summary)

# Explain volatility
request = rag_service_pb2.VolatilityRequest(symbol="AAPL", date="2024-01-15")
response = stub.ExplainVolatility(request)
print(response.explanation)
```

## 🧪 Testing

### Unit Tests

```bash
cd build
ctest
```

### Integration Tests

```bash
# Start server
./rag_agent_server

# Run client example
python3 scripts/client_example.py
```

## 🐳 Docker Deployment

### Build Docker Image

```bash
docker build -t rag-quant-trading-agent .
```

### Run with Docker Compose

```bash
docker-compose up -d
```

### Environment Variables in Docker

Create a `.env` file:

```env
ALPHA_VANTAGE_API_KEY=your_key
OPENAI_API_KEY=your_key
```

## 📈 Features

### Data Ingestion
- ✅ Real-time stock quotes
- ✅ Historical OHLCV data
- ✅ Options data
- ✅ News articles
- ✅ Company fundamentals
- ✅ Volatility calculations

### Vectorization
- ✅ OpenAI embeddings (text-embedding-3-small)
- ✅ FAISS index for similarity search
- ✅ Document metadata storage

### RAG Agent
- ✅ Context retrieval from vector store
- ✅ LLM response generation (GPT-4)
- ✅ Stock summary generation
- ✅ Volatility explanation
- ✅ Sentiment comparison
- ✅ Pair trading recommendations

### API Gateway
- ✅ gRPC service interface
- ✅ Protocol Buffers for serialization
- ✅ Error handling and logging

## 🛠️ Advanced Configuration

### Custom Embedding Models

Edit `src/vectorization/embedding_service.cpp` to use different embedding models:

```cpp
request_json["model"] = "text-embedding-ada-002"; // or other models
```

### Custom LLM Models

Edit `src/rag/rag_agent.cpp` to use different LLM models:

```cpp
request_json["model"] = "gpt-3.5-turbo"; // or other models
```

### FAISS Index Configuration

For larger datasets, consider using `IndexIVFFlat` instead of `IndexFlatL2`:

```cpp
// In faiss_index.cpp
faiss::IndexIVFFlat* index = new faiss::IndexIVFFlat(quantizer, dimension_, nlist);
```

## 📝 API Documentation

### GetStockSummary

Get a comprehensive stock summary with RAG context.

**Request:**
```protobuf
message StockSummaryRequest {
  string symbol = 1;
  string period = 2;  // e.g., "1d", "1w", "1m", "1y"
  int32 max_context_docs = 3;
}
```

**Response:**
```protobuf
message StockSummaryResponse {
  string symbol = 1;
  double current_price = 2;
  double change_percent = 3;
  double volume = 4;
  string summary = 5;
  repeated ContextDoc context_docs = 6;
  string timestamp = 7;
}
```

### ExplainVolatility

Explain why volatility spiked on a given date.

**Request:**
```protobuf
message VolatilityRequest {
  string symbol = 1;
  string date = 2;  // ISO format: YYYY-MM-DD
  int32 lookback_days = 3;
}
```

### CompareSentiment

Compare market sentiment between two tickers.

**Request:**
```protobuf
message SentimentCompareRequest {
  string ticker1 = 1;
  string ticker2 = 2;
  string period = 3;
}
```

### RecommendPair

Recommend a long/short pair trading strategy.

**Request:**
```protobuf
message PairRecommendationRequest {
  string sector = 1;
  int32 max_candidates = 2;
}
```

### QueryRAG

General RAG query with custom context.

**Request:**
```protobuf
message QueryRequest {
  string query = 1;
  int32 max_context_docs = 2;
  repeated string symbols = 3;
}
```

## 🔒 Security

- API keys are loaded from environment variables
- Use HTTPS/TLS for production deployments
- Implement authentication for gRPC services
- Sanitize user inputs
- Rate limiting for API calls

## 🚧 Roadmap

- [ ] Add Redis caching layer
- [ ] Implement streaming updates via Kafka
- [ ] Add monitoring with Prometheus
- [ ] Fine-tune models on financial datasets
- [ ] Add React.js frontend dashboard
- [ ] Support for more data sources (Polygon.io, Finnhub)
- [ ] Options chain analysis
- [ ] Backtesting framework
- [ ] Real-time alerts

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- Alpha Vantage for market data API
- OpenAI for embeddings and LLM
- Facebook Research for FAISS
- Google for gRPC and Protocol Buffers

## 📧 Contact

For questions or issues, please open an issue on GitHub.

## 🔗 References

- [Alpha Vantage API](https://www.alphavantage.co/documentation/)
- [OpenAI API](https://platform.openai.com/docs)
- [FAISS Documentation](https://github.com/facebookresearch/faiss)
- [gRPC Documentation](https://grpc.io/docs/)

