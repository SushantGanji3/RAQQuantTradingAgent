#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>
#include "data_ingestion/data_fetcher.h"

namespace rag {
namespace data {

class Database {
public:
    Database(const std::string& db_path);
    ~Database();
    
    bool initialize();
    
    // Stock data operations
    bool storeOHLCVData(const std::string& symbol, const std::vector<OHLCVData>& data);
    bool getOHLCVData(const std::string& symbol, const std::string& start_date, 
                     const std::string& end_date, std::vector<OHLCVData>& data);
    
    // Options data operations
    bool storeOptionsData(const std::vector<OptionsData>& data);
    bool getOptionsData(const std::string& symbol, std::vector<OptionsData>& data);
    
    // News operations
    bool storeNewsArticle(const NewsArticle& article);
    bool getNewsArticles(const std::string& symbol, int limit, std::vector<NewsArticle>& articles);
    bool getNewsArticlesByDate(const std::string& start_date, const std::string& end_date,
                              std::vector<NewsArticle>& articles);
    
    // Volatility operations
    bool storeVolatility(const std::string& symbol, const std::string& date, double volatility);
    bool getVolatility(const std::string& symbol, const std::string& date, double& volatility);
    
    // Fundamentals
    bool storeFundamentals(const std::string& symbol, const std::string& json_data);
    bool getFundamentals(const std::string& symbol, std::string& json_data);
    
private:
    std::string db_path_;
    sqlite3* db_;
    
    bool createTables();
    std::string escapeSQL(const std::string& str);
};

} // namespace data
} // namespace rag

