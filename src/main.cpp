#include <iostream>
#include <memory>
#include <string>
#include "data_ingestion/data_fetcher.h"
#include "data_ingestion/database.h"
#include "vectorization/embedding_service.h"
#include "vectorization/faiss_index.h"
#include "rag/rag_agent.h"
#include "api/grpc_server.h"
#include <cstdlib>
#include "utils/logger.h"

int main(int argc, char** argv) {
    // Initialize logger
    rag::utils::Logger::getInstance().setLogLevel(rag::utils::LogLevel::INFO);
    rag::utils::Logger::getInstance().setLogFile("logs/rag_agent.log");
    
    rag::utils::Logger::getInstance().info("Starting RAG Quant Trading Agent Server");
    
    // Configuration (in production, load from config file or environment variables)
    std::string data_api_key = std::getenv("ALPHA_VANTAGE_API_KEY") ? std::getenv("ALPHA_VANTAGE_API_KEY") : "";
    std::string embedding_api_key = std::getenv("OPENAI_API_KEY") ? std::getenv("OPENAI_API_KEY") : "";
    std::string llm_api_key = std::getenv("OPENAI_API_KEY") ? std::getenv("OPENAI_API_KEY") : "";
    std::string db_path = "data/trading_data.db";
    std::string faiss_index_path = "data/faiss_index.index";
    std::string server_address = "0.0.0.0:50051";
    
    if (data_api_key.empty() || embedding_api_key.empty() || llm_api_key.empty()) {
        rag::utils::Logger::getInstance().error("API keys not set. Please set ALPHA_VANTAGE_API_KEY and OPENAI_API_KEY environment variables.");
        return 1;
    }
    
    // Initialize components
    auto data_fetcher = std::make_shared<rag::data::DataFetcher>(data_api_key);
    auto database = std::make_shared<rag::data::Database>(db_path);
    
    if (!database->initialize()) {
        rag::utils::Logger::getInstance().error("Failed to initialize database");
        return 1;
    }
    
    auto embedding_service = std::make_shared<rag::vectorization::EmbeddingService>(embedding_api_key, "openai");
    auto faiss_index = std::make_shared<rag::vectorization::FAISSIndex>(1536); // OpenAI embedding dimension
    
    if (!faiss_index->initialize()) {
        rag::utils::Logger::getInstance().error("Failed to initialize FAISS index");
        return 1;
    }
    
    // Try to load existing index
    faiss_index->load(faiss_index_path);
    
    // Create RAG agent
    auto rag_agent = std::make_shared<rag::agent::RAGAgent>(
        data_fetcher, database, embedding_service, faiss_index, llm_api_key
    );
    
    rag::utils::Logger::getInstance().info("RAG Agent initialized successfully");
    
    // Start gRPC server
    rag::utils::Logger::getInstance().info("Starting gRPC server on " + server_address);
    RunServer(server_address, rag_agent);
    
    return 0;
}

