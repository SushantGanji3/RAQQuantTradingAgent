#pragma once

#include <string>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace rag {
namespace data {

struct OHLCVData {
    std::string timestamp;
    double open;
    double high;
    double low;
    double close;
    long volume;
};

struct OptionsData {
    std::string symbol;
    std::string expiry;
    double strike;
    std::string option_type;
    double bid;
    double ask;
    double implied_volatility;
    long volume;
    long open_interest;
};

struct NewsArticle {
    std::string id;
    std::string title;
    std::string content;
    std::string source;
    std::string published_time;
    std::vector<std::string> tickers;
};

class DataFetcher {
public:
    DataFetcher(const std::string& api_key);
    ~DataFetcher();
    
    // Stock data
    bool fetchStockData(const std::string& symbol, const std::string& interval, 
                       int days, std::vector<OHLCVData>& data);
    bool fetchRealTimeQuote(const std::string& symbol, double& price, double& change_percent);
    
    // Options data
    bool fetchOptionsData(const std::string& symbol, std::vector<OptionsData>& data);
    
    // News and fundamentals
    bool fetchNews(const std::string& symbol, int max_articles, std::vector<NewsArticle>& articles);
    bool fetchCompanyFundamentals(const std::string& symbol, nlohmann::json& fundamentals);
    
    // Volatility data
    bool fetchVolatility(const std::string& symbol, const std::string& date, double& volatility);
    
private:
    std::string api_key_;
    CURL* curl_handle_;
    
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* data);
    bool makeHttpRequest(const std::string& url, std::string& response);
    std::string buildAlphaVantageUrl(const std::string& function, const std::string& symbol);
    std::string buildPolygonUrl(const std::string& endpoint);
};

} // namespace data
} // namespace rag

