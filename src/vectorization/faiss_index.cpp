#include "vectorization/faiss_index.h"
#include "utils/logger.h"
#include <fstream>
#include <faiss/utils.h>

namespace rag {
namespace vectorization {

FAISSIndex::FAISSIndex(size_t dimension) : dimension_(dimension) {
    index_ = std::make_unique<faiss::IndexFlatL2>(dimension);
}

FAISSIndex::~FAISSIndex() {
}

bool FAISSIndex::initialize() {
    if (dimension_ == 0) {
        rag::utils::Logger::getInstance().error("Invalid embedding dimension: 0");
        return false;
    }
    
    rag::utils::Logger::getInstance().info("FAISS index initialized with dimension: " + std::to_string(dimension_));
    return true;
}

bool FAISSIndex::addDocument(const Document& doc, const std::vector<float>& embedding) {
    if (embedding.size() != dimension_) {
        rag::utils::Logger::getInstance().error("Embedding dimension mismatch");
        return false;
    }
    
    // Add to FAISS index
    index_->add(1, embedding.data());
    
    // Store document metadata
    documents_[doc.doc_id] = doc;
    doc_ids_.push_back(doc.doc_id);
    
    rag::utils::Logger::getInstance().debug("Added document: " + doc.doc_id);
    return true;
}

bool FAISSIndex::addDocuments(const std::vector<Document>& docs,
                              const std::vector<std::vector<float>>& embeddings) {
    if (docs.size() != embeddings.size()) {
        rag::utils::Logger::getInstance().error("Document and embedding count mismatch");
        return false;
    }
    
    // Prepare batch embedding matrix
    std::vector<float> embedding_matrix;
    for (const auto& embedding : embeddings) {
        if (embedding.size() != dimension_) {
            rag::utils::Logger::getInstance().error("Embedding dimension mismatch in batch");
            return false;
        }
        embedding_matrix.insert(embedding_matrix.end(), embedding.begin(), embedding.end());
    }
    
    // Add to FAISS index
    index_->add(docs.size(), embedding_matrix.data());
    
    // Store document metadata
    for (size_t i = 0; i < docs.size(); ++i) {
        documents_[docs[i].doc_id] = docs[i];
        doc_ids_.push_back(docs[i].doc_id);
    }
    
    rag::utils::Logger::getInstance().info("Added " + std::to_string(docs.size()) + " documents to index");
    return true;
}

std::vector<SearchResult> FAISSIndex::search(const std::vector<float>& query_embedding,
                                            size_t k) {
    std::vector<SearchResult> results;
    
    if (query_embedding.size() != dimension_) {
        rag::utils::Logger::getInstance().error("Query embedding dimension mismatch");
        return results;
    }
    
    if (index_->ntotal == 0) {
        rag::utils::Logger::getInstance().warning("Index is empty, cannot search");
        return results;
    }
    
    // Adjust k if necessary
    size_t actual_k = std::min(k, static_cast<size_t>(index_->ntotal));
    
    // Prepare output arrays
    std::vector<faiss::idx_t> indices(actual_k);
    std::vector<float> distances(actual_k);
    
    // Search
    index_->search(1, query_embedding.data(), actual_k, distances.data(), indices.data());
    
    // Convert to SearchResult
    for (size_t i = 0; i < actual_k; ++i) {
        if (indices[i] >= 0 && indices[i] < static_cast<faiss::idx_t>(doc_ids_.size())) {
            std::string doc_id = doc_ids_[indices[i]];
            if (documents_.find(doc_id) != documents_.end()) {
                SearchResult result;
                result.doc_id = doc_id;
                result.content = documents_[doc_id].content;
                result.source = documents_[doc_id].source;
                result.timestamp = documents_[doc_id].timestamp;
                result.metadata = documents_[doc_id].metadata;
                result.similarity_score = 1.0f / (1.0f + distances[i]); // Convert L2 distance to similarity
                results.push_back(result);
            }
        }
    }
    
    return results;
}

bool FAISSIndex::removeDocument(const std::string& doc_id) {
    // FAISS doesn't support efficient removal, so we'd need to rebuild the index
    // For now, just remove from metadata
    if (documents_.find(doc_id) != documents_.end()) {
        documents_.erase(doc_id);
        rag::utils::Logger::getInstance().info("Removed document metadata: " + doc_id);
        // Note: The embedding is still in the index, but won't be found in search results
        // due to missing metadata
        return true;
    }
    return false;
}

bool FAISSIndex::getDocument(const std::string& doc_id, Document& doc) {
    if (documents_.find(doc_id) != documents_.end()) {
        doc = documents_[doc_id];
        return true;
    }
    return false;
}

bool FAISSIndex::save(const std::string& filepath) {
    try {
        // Save FAISS index
        faiss::write_index(index_.get(), filepath.c_str());
        
        // Save metadata to separate file
        std::string metadata_filepath = filepath + ".meta";
        std::ofstream meta_file(metadata_filepath);
        if (!meta_file.is_open()) {
            rag::utils::Logger::getInstance().error("Failed to open metadata file for writing");
            return false;
        }
        
        // Simple serialization (in production, use protobuf or JSON)
        meta_file << doc_ids_.size() << std::endl;
        for (const auto& doc_id : doc_ids_) {
            meta_file << doc_id << std::endl;
            if (documents_.find(doc_id) != documents_.end()) {
                const auto& doc = documents_[doc_id];
                meta_file << doc.content << std::endl;
                meta_file << doc.source << std::endl;
                meta_file << doc.timestamp << std::endl;
                meta_file << doc.metadata.size() << std::endl;
                for (const auto& [key, value] : doc.metadata) {
                    meta_file << key << std::endl << value << std::endl;
                }
            }
        }
        meta_file.close();
        
        rag::utils::Logger::getInstance().info("Saved FAISS index to: " + filepath);
        return true;
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to save index: " + std::string(e.what()));
        return false;
    }
}

bool FAISSIndex::load(const std::string& filepath) {
    try {
        // Load FAISS index
        faiss::Index* loaded_index = faiss::read_index(filepath.c_str());
        if (!loaded_index) {
            rag::utils::Logger::getInstance().error("Failed to load FAISS index");
            return false;
        }
        
        index_.reset(dynamic_cast<faiss::IndexFlatL2*>(loaded_index));
        if (!index_) {
            rag::utils::Logger::getInstance().error("Loaded index is not IndexFlatL2");
            delete loaded_index;
            return false;
        }
        
        // Load metadata
        std::string metadata_filepath = filepath + ".meta";
        std::ifstream meta_file(metadata_filepath);
        if (!meta_file.is_open()) {
            rag::utils::Logger::getInstance().error("Failed to open metadata file for reading");
            return false;
        }
        
        documents_.clear();
        doc_ids_.clear();
        
        size_t doc_count;
        meta_file >> doc_count;
        meta_file.ignore(); // Skip newline
        
        for (size_t i = 0; i < doc_count; ++i) {
            std::string doc_id;
            std::getline(meta_file, doc_id);
            
            Document doc;
            doc.doc_id = doc_id;
            std::getline(meta_file, doc.content);
            std::getline(meta_file, doc.source);
            std::getline(meta_file, doc.timestamp);
            
            size_t metadata_count;
            meta_file >> metadata_count;
            meta_file.ignore();
            
            for (size_t j = 0; j < metadata_count; ++j) {
                std::string key, value;
                std::getline(meta_file, key);
                std::getline(meta_file, value);
                doc.metadata[key] = value;
            }
            
            documents_[doc_id] = doc;
            doc_ids_.push_back(doc_id);
        }
        
        meta_file.close();
        rag::utils::Logger::getInstance().info("Loaded FAISS index from: " + filepath);
        return true;
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to load index: " + std::string(e.what()));
        return false;
    }
}

size_t FAISSIndex::size() const {
    return documents_.size();
}

bool FAISSIndex::buildIndex() {
    // Index is built automatically when adding documents
    return true;
}

} // namespace vectorization
} // namespace rag

