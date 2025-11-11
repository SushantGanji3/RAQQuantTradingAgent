# API Documentation

## gRPC Service: RAGAgentService

The RAG Agent exposes a gRPC service with the following methods:

### GetStockSummary

Get a comprehensive stock summary with RAG context.

**Request**:
```protobuf
message StockSummaryRequest {
  string symbol = 1;           // Stock symbol (e.g., "AAPL")
  string period = 2;           // Time period: "1d", "1w", "1m", "1y"
  int32 max_context_docs = 3;  // Maximum number of context documents
}
```

**Response**:
```protobuf
message StockSummaryResponse {
  string symbol = 1;
  double current_price = 2;
  double change_percent = 3;
  double volume = 4;
  string summary = 5;                    // LLM-generated summary
  repeated ContextDoc context_docs = 6;  // Retrieved context documents
  string timestamp = 7;
}
```

**Example**:
```python
request = StockSummaryRequest(symbol="AAPL", period="1m", max_context_docs=5)
response = stub.GetStockSummary(request)
print(response.summary)
```

### ExplainVolatility

Explain why volatility spiked on a given date.

**Request**:
```protobuf
message VolatilityRequest {
  string symbol = 1;           // Stock symbol
  string date = 2;             // ISO format: YYYY-MM-DD
  int32 lookback_days = 3;     // Number of days to look back
}
```

**Response**:
```protobuf
message VolatilityResponse {
  string symbol = 1;
  string date = 2;
  double volatility_value = 3;
  double previous_volatility = 4;
  string explanation = 5;                // LLM-generated explanation
  repeated ContextDoc context_docs = 6;  // Relevant news and events
}
```

**Example**:
```python
request = VolatilityRequest(symbol="AAPL", date="2024-01-15", lookback_days=7)
response = stub.ExplainVolatility(request)
print(response.explanation)
```

### CompareSentiment

Compare market sentiment between two tickers.

**Request**:
```protobuf
message SentimentCompareRequest {
  string ticker1 = 1;  // First ticker
  string ticker2 = 2;  // Second ticker
  string period = 3;   // Time period
}
```

**Response**:
```protobuf
message SentimentCompareResponse {
  string ticker1 = 1;
  string ticker2 = 2;
  double sentiment1 = 3;       // Sentiment score for ticker1 (-1 to 1)
  double sentiment2 = 4;       // Sentiment score for ticker2 (-1 to 1)
  string comparison = 5;       // LLM-generated comparison
  repeated ContextDoc context_docs = 6;
}
```

**Example**:
```python
request = SentimentCompareRequest(ticker1="AAPL", ticker2="MSFT", period="1m")
response = stub.CompareSentiment(request)
print(response.comparison)
```

### RecommendPair

Recommend a long/short pair trading strategy.

**Request**:
```protobuf
message PairRecommendationRequest {
  string sector = 1;           // Sector name (e.g., "technology")
  int32 max_candidates = 2;    // Maximum number of candidate pairs
}
```

**Response**:
```protobuf
message PairRecommendationResponse {
  string long_ticker = 1;      // Ticker to go long
  string short_ticker = 2;     // Ticker to go short
  string reasoning = 3;        // LLM-generated reasoning
  double correlation = 4;      // Historical correlation
  double expected_return = 5;  // Expected return
  repeated ContextDoc context_docs = 6;
}
```

**Example**:
```python
request = PairRecommendationRequest(sector="technology", max_candidates=10)
response = stub.RecommendPair(request)
print(f"Long: {response.long_ticker}, Short: {response.short_ticker}")
print(response.reasoning)
```

### QueryRAG

General RAG query with custom context.

**Request**:
```protobuf
message QueryRequest {
  string query = 1;                    // Natural language query
  int32 max_context_docs = 2;          // Maximum context documents
  repeated string symbols = 3;         // Optional symbol filters
}
```

**Response**:
```protobuf
message QueryResponse {
  string answer = 1;                   // LLM-generated answer
  repeated ContextDoc context_docs = 2; // Retrieved context
  double confidence = 3;               // Confidence score (0-1)
  string timestamp = 4;
}
```

**Example**:
```python
request = QueryRequest(
    query="Why did NVIDIA stock surge last week?",
    max_context_docs=10,
    symbols=["NVDA"]
)
response = stub.QueryRAG(request)
print(response.answer)
```

## ContextDoc

Context documents retrieved from the vector store:

```protobuf
message ContextDoc {
  string doc_id = 1;              // Unique document ID
  string content = 2;             // Document content
  string source = 3;              // Source (e.g., "news", "filing")
  string timestamp = 4;           // Publication timestamp
  double similarity_score = 5;    // Similarity score (0-1)
  map<string, string> metadata = 6; // Additional metadata
}
```

## Error Handling

All methods return gRPC status codes:

- `OK`: Request succeeded
- `INVALID_ARGUMENT`: Invalid request parameters
- `NOT_FOUND`: Resource not found
- `INTERNAL`: Internal server error
- `UNAVAILABLE`: Service unavailable

## Rate Limiting

API calls are rate-limited to prevent abuse:
- 100 requests per minute per client
- 1000 requests per hour per client

## Authentication

Authentication is handled via API keys (future: OAuth 2.0):
- Include API key in gRPC metadata
- Key: `x-api-key`
- Value: Your API key

## Client Examples

### Python

```python
import grpc
from proto import rag_service_pb2, rag_service_pb2_grpc

# Create channel
channel = grpc.insecure_channel('localhost:50051')
stub = rag_service_pb2_grpc.RAGAgentServiceStub(channel)

# Make request
request = rag_service_pb2.StockSummaryRequest(symbol="AAPL", period="1m")
response = stub.GetStockSummary(request)
print(response.summary)
```

### C++

```cpp
#include "proto/rag_service.grpc.pb.h"
#include <grpcpp/grpcpp.h>

auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
auto stub = rag::agent::RAGAgentService::NewStub(channel);

rag::agent::StockSummaryRequest request;
request.set_symbol("AAPL");
request.set_period("1m");

rag::agent::StockSummaryResponse response;
grpc::ClientContext context;
grpc::Status status = stub->GetStockSummary(&context, request, &response);

if (status.ok()) {
    std::cout << response.summary() << std::endl;
}
```

### JavaScript/TypeScript

```typescript
import * as grpc from '@grpc/grpc-js';
import { RAGAgentServiceClient } from './proto/rag_service_grpc_pb';
import { StockSummaryRequest } from './proto/rag_service_pb';

const client = new RAGAgentServiceClient('localhost:50051', grpc.credentials.createInsecure());

const request = new StockSummaryRequest();
request.setSymbol('AAPL');
request.setPeriod('1m');

client.getStockSummary(request, (error, response) => {
    if (error) {
        console.error(error);
    } else {
        console.log(response.getSummary());
    }
});
```

## Streaming (Future)

Future versions will support streaming responses for real-time updates:

```protobuf
rpc StreamStockUpdates(StockUpdateRequest) returns (stream StockUpdateResponse);
```

