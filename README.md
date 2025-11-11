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
  - Alpha Vantage API key (for market data) - [Get one here](https://www.alphavantage.co/support/#api-key)
  - OpenAI API key (for embeddings and LLM) - [Get one here](https://platform.openai.com/api-keys)

## 🚀 Quick Start

### Option 1: Quick Setup (Recommended)

```bash
# Clone repository
git clone https://github.com/SushantGanji3/RAQQuantTradingAgent.git
cd RAQQuantTradingAgent

# Run setup script
chmod +x setup.sh
./setup.sh

# Set API keys (create .env file)
cp .env.example .env
# Edit .env and add your API keys

# Build and run
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
./rag_agent_server
```

### Option 2: Manual Setup

See [GETTING_STARTED.md](GETTING_STARTED.md) for detailed step-by-step instructions.

## 📦 Installation

### macOS

```bash
# Install dependencies
./scripts/install_macos_deps.sh

# Or manually:
brew install cmake curl sqlite3 pkg-config
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
cd ..
./vcpkg/vcpkg install curl nlohmann-json sqlite3 grpc protobuf abseil

# Install FAISS (recommended: use Conda)
conda install -c pytorch faiss-cpu
```

### Linux

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev \
    libsqlite3-dev pkg-config python3 python3-pip

# Install vcpkg and dependencies
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install curl nlohmann-json sqlite3 grpc protobuf abseil
```

For detailed installation instructions, see [INSTALL_DEPENDENCIES.md](INSTALL_DEPENDENCIES.md).

## 🔧 Configuration

### API Keys

Create a `.env` file with your API keys:

```bash
cp .env.example .env
# Edit .env and add your keys:
# ALPHA_VANTAGE_API_KEY=your_key
# OPENAI_API_KEY=your_key
```

Or set environment variables:

```bash
export ALPHA_VANTAGE_API_KEY="your_key"
export OPENAI_API_KEY="your_key"
```

**Note**: The `.env` file is in `.gitignore` and will not be committed to the repository.

## 🏃 Running the Server

### Method 1: Using the Run Script

```bash
# Load environment and run server
./scripts/run_server.sh
```

### Method 2: Manual

```bash
# Load environment variables
source scripts/load_env.sh

# Run server
cd build
./rag_agent_server
```

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

## 🔌 API Endpoints

The gRPC server exposes the following services:

- `GetStockSummary`: Get stock summary with context
- `ExplainVolatility`: Explain volatility spike
- `CompareSentiment`: Compare sentiment between two tickers
- `RecommendPair`: Recommend long/short pair
- `QueryRAG`: General RAG query

For detailed API documentation, see [docs/API.md](docs/API.md).

## ✨ Features

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

## 📁 Project Structure

```
RAQQuantTradingAgent/
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
├── docs/                   # Documentation
│   ├── ARCHITECTURE.md
│   ├── API.md
│   ├── BUILD.md
│   └── DEPLOYMENT.md
└── README.md
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

## 📚 Documentation

- [QUICKSTART.md](QUICKSTART.md) - Quick start guide
- [GETTING_STARTED.md](GETTING_STARTED.md) - Getting started guide
- [INSTALL_DEPENDENCIES.md](INSTALL_DEPENDENCIES.md) - Dependency installation
- [API_KEYS_SETUP.md](API_KEYS_SETUP.md) - API keys configuration
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) - System architecture
- [docs/API.md](docs/API.md) - API documentation
- [docs/BUILD.md](docs/BUILD.md) - Build instructions
- [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md) - Deployment guide

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

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed contribution guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Alpha Vantage for market data API
- OpenAI for embeddings and LLM
- Facebook Research for FAISS
- Google for gRPC and Protocol Buffers

## 📧 Contact

For questions or issues, please open an issue on [GitHub](https://github.com/SushantGanji3/RAQQuantTradingAgent/issues).

## 🔗 References

- [Alpha Vantage API](https://www.alphavantage.co/documentation/)
- [OpenAI API](https://platform.openai.com/docs)
- [FAISS Documentation](https://github.com/facebookresearch/faiss)
- [gRPC Documentation](https://grpc.io/docs/)

---

**Repository**: https://github.com/SushantGanji3/RAQQuantTradingAgent  
**License**: MIT  
**Status**: Active Development

