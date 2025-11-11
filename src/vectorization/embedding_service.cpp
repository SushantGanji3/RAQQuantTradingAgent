#include "vectorization/embedding_service.h"
#include "utils/logger.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>

namespace rag {
namespace vectorization {

EmbeddingService::EmbeddingService(const std::string& api_key, const std::string& provider)
    : api_key_(api_key), provider_(provider), embedding_dimension_(1536) {
    if (provider == "vertex") {
        embedding_dimension_ = 768; // Vertex AI embedding dimension
    }
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t total_size = size * nmemb;
    data->append((char*)contents, total_size);
    return total_size;
}

bool EmbeddingService::generateEmbedding(const std::string& text, std::vector<float>& embedding) {
    if (provider_ == "openai") {
        return generateOpenAIEmbedding(text, embedding);
    } else if (provider_ == "vertex") {
        return generateVertexAIEmbedding(text, embedding);
    }
    return false;
}

bool EmbeddingService::generateOpenAIEmbedding(const std::string& text, std::vector<float>& embedding) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        rag::utils::Logger::getInstance().error("Failed to initialize CURL for embedding");
        return false;
    }
    
    nlohmann::json request_json;
    request_json["input"] = text;
    request_json["model"] = "text-embedding-3-small"; // or text-embedding-ada-002
    
    std::string json_str = request_json.dump();
    std::string response;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Bearer " + api_key_;
    headers = curl_slist_append(headers, auth_header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/embeddings");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        rag::utils::Logger::getInstance().error("CURL request failed for embedding");
        return false;
    }
    
    try {
        nlohmann::json json_response = nlohmann::json::parse(response);
        if (json_response.contains("data") && json_response["data"].is_array() && 
            !json_response["data"].empty()) {
            auto embedding_data = json_response["data"][0]["embedding"];
            embedding.clear();
            for (const auto& val : embedding_data) {
                embedding.push_back(val.get<float>());
            }
            embedding_dimension_ = embedding.size();
            return true;
        }
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse embedding response: " + std::string(e.what()));
    }
    
    return false;
}

bool EmbeddingService::generateVertexAIEmbedding(const std::string& text, std::vector<float>& embedding) {
    // Vertex AI embedding implementation
    // This would use Google Cloud Vertex AI API
    rag::utils::Logger::getInstance().info("Vertex AI embeddings not fully implemented yet");
    return false;
}

bool EmbeddingService::generateEmbeddings(const std::vector<std::string>& texts,
                                         std::vector<std::vector<float>>& embeddings) {
    embeddings.clear();
    for (const auto& text : texts) {
        std::vector<float> embedding;
        if (!generateEmbedding(text, embedding)) {
            return false;
        }
        embeddings.push_back(embedding);
    }
    return true;
}

} // namespace vectorization
} // namespace rag

