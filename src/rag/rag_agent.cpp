#include "rag/rag_agent.h"
#include "utils/logger.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>

namespace rag {
namespace agent {

RAGAgent::RAGAgent(std::shared_ptr<data::DataFetcher> data_fetcher,
                   std::shared_ptr<data::Database> database,
                   std::shared_ptr<vectorization::EmbeddingService> embedding_service,
                   std::shared_ptr<vectorization::FAISSIndex> faiss_index,
                   const std::string& llm_api_key)
    : data_fetcher_(data_fetcher),
      database_(database),
      embedding_service_(embedding_service),
      faiss_index_(faiss_index),
      llm_api_key_(llm_api_key) {
}

std::vector<ContextDoc> RAGAgent::retrieveContext(const std::string& query, size_t k) {
    std::vector<ContextDoc> context_docs;
    
    // Generate embedding for query
    std::vector<float> query_embedding;
    if (!embedding_service_->generateEmbedding(query, query_embedding)) {
        rag::utils::Logger::getInstance().error("Failed to generate query embedding");
        return context_docs;
    }
    
    // Search FAISS index
    auto search_results = faiss_index_->search(query_embedding, k);
    
    // Convert to ContextDoc
    for (const auto& result : search_results) {
        ContextDoc doc;
        doc.doc_id = result.doc_id;
        doc.content = result.content;
        doc.source = result.source;
        doc.timestamp = result.timestamp;
        doc.similarity_score = result.similarity_score;
        doc.metadata = result.metadata;
        context_docs.push_back(doc);
    }
    
    return context_docs;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t total_size = size * nmemb;
    data->append((char*)contents, total_size);
    return total_size;
}

std::string RAGAgent::queryOpenAI(const std::string& prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        rag::utils::Logger::getInstance().error("Failed to initialize CURL for LLM");
        return "";
    }
    
    nlohmann::json request_json;
    request_json["model"] = "gpt-4";
    request_json["messages"] = nlohmann::json::array();
    request_json["messages"].push_back({{"role", "system"}, {"content", "You are a quantitative trading analyst assistant. Provide concise, data-driven insights based on the provided context."}});
    request_json["messages"].push_back({{"role", "user"}, {"content", prompt}});
    request_json["temperature"] = 0.7;
    request_json["max_tokens"] = 1000;
    
    std::string json_str = request_json.dump();
    std::string response;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + llm_api_key_;
    headers = curl_slist_append(headers, auth_header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        rag::utils::Logger::getInstance().error("CURL request failed for LLM");
        return "";
    }
    
    try {
        nlohmann::json json_response = nlohmann::json::parse(response);
        if (json_response.contains("choices") && json_response["choices"].is_array() &&
            !json_response["choices"].empty()) {
            return json_response["choices"][0]["message"]["content"].get<std::string>();
        }
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse LLM response: " + std::string(e.what()));
    }
    
    return "";
}

std::string RAGAgent::generateLLMResponse(const std::string& query,
                                         const std::vector<ContextDoc>& context_docs) {
    // Build prompt with context
    std::stringstream prompt_ss;
    prompt_ss << "Query: " << query << "\n\n";
    prompt_ss << "Context from financial data and news:\n";
    
    for (size_t i = 0; i < context_docs.size(); ++i) {
        prompt_ss << "\n[Document " << (i + 1) << "]\n";
        prompt_ss << "Source: " << context_docs[i].source << "\n";
        prompt_ss << "Timestamp: " << context_docs[i].timestamp << "\n";
        prompt_ss << "Content: " << context_docs[i].content << "\n";
    }
    
    prompt_ss << "\n\nBased on the above context, please provide a comprehensive answer to the query.\n";
    
    return queryOpenAI(prompt_ss.str());
}

bool RAGAgent::getStockSummary(const std::string& symbol, const std::string& period,
                              std::string& summary, std::vector<ContextDoc>& context_docs) {
    // Fetch current stock data
    double price, change_percent;
    if (!data_fetcher_->fetchRealTimeQuote(symbol, price, change_percent)) {
        rag::utils::Logger::getInstance().error("Failed to fetch stock quote for " + symbol);
        return false;
    }
    
    // Retrieve relevant context
    std::string query = "Stock summary for " + symbol + " over " + period;
    context_docs = retrieveContext(query, 5);
    
    // Build query with stock data
    std::stringstream query_ss;
    query_ss << "Provide a summary for " << symbol << " stock. ";
    query_ss << "Current price: $" << price << " (" << change_percent << "%). ";
    query_ss << "Period: " << period << ". ";
    query_ss << "Include key metrics, recent news, and market sentiment.";
    
    summary = generateLLMResponse(query_ss.str(), context_docs);
    return !summary.empty();
}

bool RAGAgent::explainVolatility(const std::string& symbol, const std::string& date,
                                 std::string& explanation, std::vector<ContextDoc>& context_docs) {
    // Fetch volatility data
    double volatility;
    if (!data_fetcher_->fetchVolatility(symbol, date, volatility)) {
        rag::utils::Logger::getInstance().error("Failed to fetch volatility for " + symbol);
        return false;
    }
    
    // Retrieve relevant context (news, events around that date)
    std::string query = "Volatility spike " + symbol + " " + date;
    context_docs = retrieveContext(query, 10);
    
    // Fetch news around that date
    std::vector<data::NewsArticle> articles;
    data_fetcher_->fetchNews(symbol, 10, articles);
    
    // Build query
    std::stringstream query_ss;
    query_ss << "Explain why " << symbol << " volatility was " << volatility;
    query_ss << " on " << date << ". Consider news, events, and market conditions.";
    
    explanation = generateLLMResponse(query_ss.str(), context_docs);
    return !explanation.empty();
}

bool RAGAgent::compareSentiment(const std::string& ticker1, const std::string& ticker2,
                                const std::string& period, std::string& comparison,
                                std::vector<ContextDoc>& context_docs) {
    // Retrieve context for both tickers
    std::string query = "Sentiment comparison " + ticker1 + " " + ticker2 + " " + period;
    context_docs = retrieveContext(query, 10);
    
    // Fetch news for both
    std::vector<data::NewsArticle> articles1, articles2;
    data_fetcher_->fetchNews(ticker1, 10, articles1);
    data_fetcher_->fetchNews(ticker2, 10, articles2);
    
    // Build query
    std::stringstream query_ss;
    query_ss << "Compare market sentiment between " << ticker1 << " and " << ticker2;
    query_ss << " over " << period << ". Include news sentiment, analyst opinions, and price trends.";
    
    comparison = generateLLMResponse(query_ss.str(), context_docs);
    return !comparison.empty();
}

bool RAGAgent::recommendPair(const std::string& sector, std::string& long_ticker,
                            std::string& short_ticker, std::string& reasoning,
                            std::vector<ContextDoc>& context_docs) {
    // Retrieve context for sector
    std::string query = "Pair trading recommendation " + sector;
    context_docs = retrieveContext(query, 10);
    
    // Build query
    std::stringstream query_ss;
    query_ss << "Recommend a long/short pair trading strategy for the " << sector << " sector. ";
    query_ss << "Identify one stock to go long and one to go short, with reasoning based on fundamentals, ";
    query_ss << "technical analysis, and market sentiment.";
    
    std::string response = generateLLMResponse(query_ss.str(), context_docs);
    
    // Parse response (simplified - in production, use structured output)
    // For now, just return the response as reasoning
    reasoning = response;
    long_ticker = "AAPL"; // Placeholder
    short_ticker = "MSFT"; // Placeholder
    
    return !reasoning.empty();
}

bool RAGAgent::queryRAG(const std::string& query, const std::vector<std::string>& symbols,
                       std::string& answer, std::vector<ContextDoc>& context_docs) {
    // Retrieve relevant context
    context_docs = retrieveContext(query, 10);
    
    // Generate answer with context
    answer = generateLLMResponse(query, context_docs);
    return !answer.empty();
}

} // namespace agent
} // namespace rag

