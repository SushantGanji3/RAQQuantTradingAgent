#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "data_ingestion/data_fetcher.h"
#include "data_ingestion/database.h"
#include "vectorization/embedding_service.h"
#include "vectorization/faiss_index.h"

namespace rag {
namespace agent {

struct ContextDoc {
    std::string doc_id;
    std::string content;
    std::string source;
    std::string timestamp;
    double similarity_score;
    std::map<std::string, std::string> metadata;
};

class RAGAgent {
public:
    RAGAgent(std::shared_ptr<data::DataFetcher> data_fetcher,
             std::shared_ptr<data::Database> database,
             std::shared_ptr<vectorization::EmbeddingService> embedding_service,
             std::shared_ptr<vectorization::FAISSIndex> faiss_index,
             const std::string& llm_api_key);
    
    // Get stock summary with RAG context
    bool getStockSummary(const std::string& symbol, const std::string& period,
                        std::string& summary, std::vector<ContextDoc>& context_docs);
    
    // Explain volatility spike
    bool explainVolatility(const std::string& symbol, const std::string& date,
                          std::string& explanation, std::vector<ContextDoc>& context_docs);
    
    // Compare sentiment between two tickers
    bool compareSentiment(const std::string& ticker1, const std::string& ticker2,
                         const std::string& period, std::string& comparison,
                         std::vector<ContextDoc>& context_docs);
    
    // Recommend long/short pair
    bool recommendPair(const std::string& sector, std::string& long_ticker,
                      std::string& short_ticker, std::string& reasoning,
                      std::vector<ContextDoc>& context_docs);
    
    // General RAG query
    bool queryRAG(const std::string& query, const std::vector<std::string>& symbols,
                 std::string& answer, std::vector<ContextDoc>& context_docs);
    
private:
    std::shared_ptr<data::DataFetcher> data_fetcher_;
    std::shared_ptr<data::Database> database_;
    std::shared_ptr<vectorization::EmbeddingService> embedding_service_;
    std::shared_ptr<vectorization::FAISSIndex> faiss_index_;
    std::string llm_api_key_;
    
    // Retrieve relevant context from vector store
    std::vector<ContextDoc> retrieveContext(const std::string& query, size_t k = 5);
    
    // Generate LLM response with context
    std::string generateLLMResponse(const std::string& query, 
                                   const std::vector<ContextDoc>& context_docs);
    
    // Query OpenAI GPT API
    std::string queryOpenAI(const std::string& prompt);
};

} // namespace agent
} // namespace rag

