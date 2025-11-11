#pragma once

#include <string>
#include <vector>
#include <memory>

namespace rag {
namespace vectorization {

class EmbeddingService {
public:
    EmbeddingService(const std::string& api_key, const std::string& provider = "openai");
    ~EmbeddingService() = default;
    
    // Generate embeddings for text
    bool generateEmbedding(const std::string& text, std::vector<float>& embedding);
    
    // Generate embeddings for multiple texts (batch)
    bool generateEmbeddings(const std::vector<std::string>& texts, 
                           std::vector<std::vector<float>>& embeddings);
    
    // Get embedding dimension
    size_t getEmbeddingDimension() const { return embedding_dimension_; }
    
private:
    std::string api_key_;
    std::string provider_;
    size_t embedding_dimension_;
    
    bool generateOpenAIEmbedding(const std::string& text, std::vector<float>& embedding);
    bool generateVertexAIEmbedding(const std::string& text, std::vector<float>& embedding);
};

} // namespace vectorization
} // namespace rag

