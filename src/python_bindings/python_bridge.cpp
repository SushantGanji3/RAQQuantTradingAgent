#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "rag/rag_agent.h"
#include "data_ingestion/data_fetcher.h"
#include "data_ingestion/database.h"
#include "vectorization/embedding_service.h"
#include "vectorization/faiss_index.h"

namespace py = pybind11;

PYBIND11_MODULE(rag_agent_py, m) {
    m.doc() = "RAG Quant Trading Agent Python Bindings";
    
    // DataFetcher
    py::class_<rag::data::DataFetcher, std::shared_ptr<rag::data::DataFetcher>>(m, "DataFetcher")
        .def(py::init<const std::string&>())
        .def("fetch_stock_data", &rag::data::DataFetcher::fetchStockData)
        .def("fetch_real_time_quote", &rag::data::DataFetcher::fetchRealTimeQuote)
        .def("fetch_news", &rag::data::DataFetcher::fetchNews);
    
    // Database
    py::class_<rag::data::Database, std::shared_ptr<rag::data::Database>>(m, "Database")
        .def(py::init<const std::string&>())
        .def("initialize", &rag::data::Database::initialize)
        .def("store_ohlcv_data", &rag::data::Database::storeOHLCVData)
        .def("get_ohlcv_data", &rag::data::Database::getOHLCVData)
        .def("store_news_article", &rag::data::Database::storeNewsArticle)
        .def("get_news_articles", &rag::data::Database::getNewsArticles);
    
    // EmbeddingService
    py::class_<rag::vectorization::EmbeddingService, std::shared_ptr<rag::vectorization::EmbeddingService>>(m, "EmbeddingService")
        .def(py::init<const std::string&, const std::string&>())
        .def("generate_embedding", &rag::vectorization::EmbeddingService::generateEmbedding)
        .def("generate_embeddings", &rag::vectorization::EmbeddingService::generateEmbeddings);
    
    // FAISSIndex
    py::class_<rag::vectorization::FAISSIndex, std::shared_ptr<rag::vectorization::FAISSIndex>>(m, "FAISSIndex")
        .def(py::init<size_t>())
        .def("initialize", &rag::vectorization::FAISSIndex::initialize)
        .def("add_document", &rag::vectorization::FAISSIndex::addDocument)
        .def("search", &rag::vectorization::FAISSIndex::search)
        .def("save", &rag::vectorization::FAISSIndex::save)
        .def("load", &rag::vectorization::FAISSIndex::load);
    
    // RAGAgent
    py::class_<rag::agent::RAGAgent, std::shared_ptr<rag::agent::RAGAgent>>(m, "RAGAgent")
        .def(py::init<std::shared_ptr<rag::data::DataFetcher>,
                     std::shared_ptr<rag::data::Database>,
                     std::shared_ptr<rag::vectorization::EmbeddingService>,
                     std::shared_ptr<rag::vectorization::FAISSIndex>,
                     const std::string&>())
        .def("get_stock_summary", &rag::agent::RAGAgent::getStockSummary)
        .def("explain_volatility", &rag::agent::RAGAgent::explainVolatility)
        .def("compare_sentiment", &rag::agent::RAGAgent::compareSentiment)
        .def("recommend_pair", &rag::agent::RAGAgent::recommendPair)
        .def("query_rag", &rag::agent::RAGAgent::queryRAG);
}

