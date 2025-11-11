# Architecture Documentation

## System Architecture

The RAG Quant Trading Agent is built with a modular architecture that separates concerns into distinct layers:

### 1. Data Ingestion Layer

**Purpose**: Fetch and store real-time and historical financial data efficiently.

**Components**:
- `DataFetcher`: Handles HTTP requests to financial APIs (Alpha Vantage, Polygon.io)
- `Database`: SQLite-based storage for OHLCV data, news, options, and fundamentals

**Key Features**:
- Async data fetching with libcurl
- Efficient batch storage with transactions
- Timestamp-based data organization
- Support for multiple data sources

### 2. Vectorization & Retrieval Layer

**Purpose**: Store financial documents in a vector database for semantic search.

**Components**:
- `EmbeddingService`: Generates embeddings using OpenAI or Vertex AI
- `FAISSIndex`: Stores and searches vector embeddings

**Key Features**:
- Semantic search over financial documents
- Efficient similarity search with FAISS
- Metadata preservation for context
- Batch embedding generation

### 3. RAG Agent Layer

**Purpose**: Generate insights using LLM augmented by retrieved context.

**Components**:
- `RAGAgent`: Orchestrates retrieval and generation
- Context retrieval from vector store
- LLM query generation with context

**Key Features**:
- Context-aware response generation
- Multiple query types (summary, volatility, sentiment, etc.)
- Configurable context window
- Confidence scoring

### 4. API Gateway Layer

**Purpose**: Serve user requests through a structured interface.

**Components**:
- gRPC server with Protocol Buffers
- Service implementations for each query type
- Error handling and logging

**Key Features**:
- Type-safe API with Protocol Buffers
- Efficient binary serialization
- Streaming support (future)
- Authentication (future)

## Data Flow

```
User Query
    ↓
gRPC API Gateway
    ↓
RAG Agent
    ↓
┌─────────────┬─────────────┐
│   Retrieve  │   Fetch     │
│   Context   │   Live Data │
│  (FAISS)    │  (API/DB)   │
└──────┬──────┴──────┬──────┘
       ↓             ↓
   Combine Context + Data
       ↓
   Generate LLM Prompt
       ↓
   Query LLM (OpenAI)
       ↓
   Return Response
```

## Component Interactions

### Data Ingestion Flow

1. `DataFetcher` makes HTTP request to financial API
2. JSON response is parsed using nlohmann/json
3. Data is cleaned and validated
4. `Database` stores data in SQLite with timestamps
5. News articles are also stored for vectorization

### Vectorization Flow

1. News articles and documents are retrieved from database
2. `EmbeddingService` generates embeddings for each document
3. Embeddings are stored in `FAISSIndex` with metadata
4. Documents are indexed for fast similarity search

### RAG Query Flow

1. User query is received via gRPC
2. Query embedding is generated
3. Similar documents are retrieved from FAISS index
4. Live data is fetched from database/API
5. Context is combined with query
6. LLM prompt is generated
7. LLM response is returned to user

## Storage Schema

### SQLite Database

**ohlcv_data**:
- symbol, timestamp, open, high, low, close, volume

**news_articles**:
- article_id, title, content, source, published_time, symbol

**volatility**:
- symbol, date, volatility

**fundamentals**:
- symbol, data (JSON), updated_at

**options_data**:
- symbol, expiry, strike, option_type, bid, ask, implied_volatility

### FAISS Index

- Vector embeddings (1536 dimensions for OpenAI)
- Document metadata (doc_id, content, source, timestamp)
- Similarity search using L2 distance

## Performance Considerations

### Optimization Strategies

1. **Batch Processing**: Process multiple documents in batches
2. **Caching**: Cache embeddings and LLM responses (future)
3. **Indexing**: Use IVFFlat index for larger datasets
4. **Async Operations**: Use async/await for I/O operations
5. **Connection Pooling**: Reuse HTTP connections

### Scalability

- Horizontal scaling with multiple server instances
- Load balancing for gRPC services
- Distributed FAISS indices (future)
- Redis cache for frequent queries
- Kafka for real-time data streaming

## Security Considerations

1. **API Keys**: Stored in environment variables
2. **Authentication**: OAuth 2.0 for API access (future)
3. **Input Validation**: Sanitize all user inputs
4. **Rate Limiting**: Limit API calls to prevent abuse
5. **TLS**: Use HTTPS/TLS for all network communications

## Error Handling

- Graceful degradation when APIs are unavailable
- Retry logic for transient failures
- Comprehensive logging for debugging
- User-friendly error messages
- Fallback responses when LLM is unavailable

## Monitoring & Logging

- Structured logging with log levels
- Performance metrics (response time, throughput)
- Error tracking and alerting
- API usage statistics
- Vector index statistics

## Future Enhancements

1. **Streaming**: Real-time data streaming with Kafka
2. **Caching**: Redis cache for frequently accessed data
3. **Monitoring**: Prometheus metrics and Grafana dashboards
4. **Frontend**: React.js dashboard for visualization
5. **Fine-tuning**: Custom models trained on financial data
6. **Multi-modal**: Support for charts and images
7. **Backtesting**: Historical strategy testing
8. **Alerts**: Real-time price and volatility alerts

