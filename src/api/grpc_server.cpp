#include "rag/rag_agent.h"
#include "utils/logger.h"
#include "grpc_server.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

// Generated proto files (created during build in build/generated/)
#include "rag_service.pb.h"
#include "rag_service.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using rag::agent::RAGAgentService;
using rag::agent::StockSummaryRequest;
using rag::agent::StockSummaryResponse;
using rag::agent::VolatilityRequest;
using rag::agent::VolatilityResponse;
using rag::agent::SentimentCompareRequest;
using rag::agent::SentimentCompareResponse;
using rag::agent::PairRecommendationRequest;
using rag::agent::PairRecommendationResponse;
using rag::agent::QueryRequest;
using rag::agent::QueryResponse;
using rag::agent::ContextDoc;

class RAGAgentServiceImpl final : public RAGAgentService::Service {
public:
    RAGAgentServiceImpl(std::shared_ptr<rag::agent::RAGAgent> rag_agent)
        : rag_agent_(rag_agent) {
    }
    
    Status GetStockSummary(ServerContext* context, const StockSummaryRequest* request,
                          StockSummaryResponse* response) override {
        rag::utils::Logger::getInstance().info("GetStockSummary request for: " + request->symbol());
        
        std::string summary;
        std::vector<rag::agent::RAGContextDoc> context_docs;
        
        if (!rag_agent_->getStockSummary(request->symbol(), request->period(),
                                        summary, context_docs)) {
            return Status(grpc::StatusCode::INTERNAL, "Failed to get stock summary");
        }
        
        response->set_symbol(request->symbol());
        response->set_summary(summary);
        
        // Add context docs (convert from RAGContextDoc to protobuf ContextDoc)
        for (const auto& doc : context_docs) {
            ContextDoc* proto_doc = response->add_context_docs();
            proto_doc->set_doc_id(doc.doc_id);
            proto_doc->set_content(doc.content);
            proto_doc->set_source(doc.source);
            proto_doc->set_timestamp(doc.timestamp);
            proto_doc->set_similarity_score(doc.similarity_score);
            for (const auto& [key, value] : doc.metadata) {
                (*proto_doc->mutable_metadata())[key] = value;
            }
        }
        
        return Status::OK;
    }
    
    Status ExplainVolatility(ServerContext* context, const VolatilityRequest* request,
                            VolatilityResponse* response) override {
        rag::utils::Logger::getInstance().info("ExplainVolatility request for: " + request->symbol());
        
        std::string explanation;
        std::vector<rag::agent::RAGContextDoc> context_docs;
        
        if (!rag_agent_->explainVolatility(request->symbol(), request->date(),
                                          explanation, context_docs)) {
            return Status(grpc::StatusCode::INTERNAL, "Failed to explain volatility");
        }
        
        response->set_symbol(request->symbol());
        response->set_date(request->date());
        response->set_explanation(explanation);
        
        // Add context docs (convert from RAGContextDoc to protobuf ContextDoc)
        for (const auto& doc : context_docs) {
            ContextDoc* proto_doc = response->add_context_docs();
            proto_doc->set_doc_id(doc.doc_id);
            proto_doc->set_content(doc.content);
            proto_doc->set_source(doc.source);
            proto_doc->set_timestamp(doc.timestamp);
            proto_doc->set_similarity_score(doc.similarity_score);
            for (const auto& [key, value] : doc.metadata) {
                (*proto_doc->mutable_metadata())[key] = value;
            }
        }
        
        return Status::OK;
    }
    
    Status CompareSentiment(ServerContext* context, const SentimentCompareRequest* request,
                           SentimentCompareResponse* response) override {
        rag::utils::Logger::getInstance().info("CompareSentiment request for: " + 
                                               request->ticker1() + " vs " + request->ticker2());
        
        std::string comparison;
        std::vector<rag::agent::RAGContextDoc> context_docs;
        
        if (!rag_agent_->compareSentiment(request->ticker1(), request->ticker2(),
                                         request->period(), comparison, context_docs)) {
            return Status(grpc::StatusCode::INTERNAL, "Failed to compare sentiment");
        }
        
        response->set_ticker1(request->ticker1());
        response->set_ticker2(request->ticker2());
        response->set_comparison(comparison);
        
        // Add context docs (convert from RAGContextDoc to protobuf ContextDoc)
        for (const auto& doc : context_docs) {
            ContextDoc* proto_doc = response->add_context_docs();
            proto_doc->set_doc_id(doc.doc_id);
            proto_doc->set_content(doc.content);
            proto_doc->set_source(doc.source);
            proto_doc->set_timestamp(doc.timestamp);
            proto_doc->set_similarity_score(doc.similarity_score);
            for (const auto& [key, value] : doc.metadata) {
                (*proto_doc->mutable_metadata())[key] = value;
            }
        }
        
        return Status::OK;
    }
    
    Status RecommendPair(ServerContext* context, const PairRecommendationRequest* request,
                        PairRecommendationResponse* response) override {
        rag::utils::Logger::getInstance().info("RecommendPair request for sector: " + request->sector());
        
        std::string long_ticker, short_ticker, reasoning;
        std::vector<rag::agent::RAGContextDoc> context_docs;
        
        if (!rag_agent_->recommendPair(request->sector(), long_ticker, short_ticker,
                                      reasoning, context_docs)) {
            return Status(grpc::StatusCode::INTERNAL, "Failed to recommend pair");
        }
        
        response->set_long_ticker(long_ticker);
        response->set_short_ticker(short_ticker);
        response->set_reasoning(reasoning);
        
        // Add context docs (convert from RAGContextDoc to protobuf ContextDoc)
        for (const auto& doc : context_docs) {
            ContextDoc* proto_doc = response->add_context_docs();
            proto_doc->set_doc_id(doc.doc_id);
            proto_doc->set_content(doc.content);
            proto_doc->set_source(doc.source);
            proto_doc->set_timestamp(doc.timestamp);
            proto_doc->set_similarity_score(doc.similarity_score);
            for (const auto& [key, value] : doc.metadata) {
                (*proto_doc->mutable_metadata())[key] = value;
            }
        }
        
        return Status::OK;
    }
    
    Status QueryRAG(ServerContext* context, const QueryRequest* request,
                   QueryResponse* response) override {
        rag::utils::Logger::getInstance().info("QueryRAG request: " + request->query());
        
        std::vector<std::string> symbols(request->symbols().begin(), request->symbols().end());
        std::string answer;
        std::vector<rag::agent::RAGContextDoc> context_docs;
        
        if (!rag_agent_->queryRAG(request->query(), symbols, answer, context_docs)) {
            return Status(grpc::StatusCode::INTERNAL, "Failed to process RAG query");
        }
        
        response->set_answer(answer);
        response->set_confidence(0.85); // Placeholder
        
        // Add context docs (convert from RAGContextDoc to protobuf ContextDoc)
        for (const auto& doc : context_docs) {
            ContextDoc* proto_doc = response->add_context_docs();
            proto_doc->set_doc_id(doc.doc_id);
            proto_doc->set_content(doc.content);
            proto_doc->set_source(doc.source);
            proto_doc->set_timestamp(doc.timestamp);
            proto_doc->set_similarity_score(doc.similarity_score);
            for (const auto& [key, value] : doc.metadata) {
                (*proto_doc->mutable_metadata())[key] = value;
            }
        }
        
        return Status::OK;
    }
    
private:
    std::shared_ptr<rag::agent::RAGAgent> rag_agent_;
};

void RunServer(const std::string& server_address, std::shared_ptr<rag::agent::RAGAgent> rag_agent) {
    RAGAgentServiceImpl service(rag_agent);
    
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    rag::utils::Logger::getInstance().info("Server listening on " + server_address);
    
    server->Wait();
}

