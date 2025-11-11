#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFFlat.h>

namespace rag {
namespace vectorization {

struct Document {
    std::string doc_id;
    std::string content;
    std::string source;
    std::string timestamp;
    std::map<std::string, std::string> metadata;
};

struct SearchResult {
    std::string doc_id;
    std::string content;
    std::string source;
    std::string timestamp;
    double similarity_score;
    std::map<std::string, std::string> metadata;
};

class FAISSIndex {
public:
    FAISSIndex(size_t dimension);
    ~FAISSIndex();
    
    // Initialize index
    bool initialize();
    
    // Add document to index
    bool addDocument(const Document& doc, const std::vector<float>& embedding);
    
    // Add multiple documents
    bool addDocuments(const std::vector<Document>& docs, 
                     const std::vector<std::vector<float>>& embeddings);
    
    // Search for similar documents
    std::vector<SearchResult> search(const std::vector<float>& query_embedding, 
                                    size_t k = 10);
    
    // Remove document from index
    bool removeDocument(const std::string& doc_id);
    
    // Get document by ID
    bool getDocument(const std::string& doc_id, Document& doc);
    
    // Save index to disk
    bool save(const std::string& filepath);
    
    // Load index from disk
    bool load(const std::string& filepath);
    
    // Get total number of documents
    size_t size() const;
    
private:
    size_t dimension_;
    std::unique_ptr<faiss::IndexFlatL2> index_;
    std::map<std::string, Document> documents_; // doc_id -> Document
    std::vector<std::string> doc_ids_; // Vector index -> doc_id mapping
    
    bool buildIndex();
};

} // namespace vectorization
} // namespace rag

