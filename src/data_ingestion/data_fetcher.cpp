#include "data_ingestion/data_fetcher.h"
#include "utils/logger.h"
#include <sstream>
#include <iostream>

namespace rag {
namespace data {

DataFetcher::DataFetcher(const std::string& api_key) : api_key_(api_key) {
    curl_handle_ = curl_easy_init();
    if (!curl_handle_) {
        rag::utils::Logger::getInstance().error("Failed to initialize CURL");
    }
}

DataFetcher::~DataFetcher() {
    if (curl_handle_) {
        curl_easy_cleanup(curl_handle_);
    }
}

size_t DataFetcher::writeCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t total_size = size * nmemb;
    data->append((char*)contents, total_size);
    return total_size;
}

bool DataFetcher::makeHttpRequest(const std::string& url, std::string& response) {
    if (!curl_handle_) {
        return false;
    }
    
    std::string read_buffer;
    
    curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle_, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl_handle_);
    
    if (res != CURLE_OK) {
        rag::utils::Logger::getInstance().error("CURL request failed: " + std::string(curl_easy_strerror(res)));
        return false;
    }
    
    long response_code;
    curl_easy_getinfo(curl_handle_, CURLINFO_RESPONSE_CODE, &response_code);
    
    if (response_code != 200) {
        rag::utils::Logger::getInstance().error("HTTP request failed with code: " + std::to_string(response_code));
        return false;
    }
    
    response = read_buffer;
    return true;
}

std::string DataFetcher::buildAlphaVantageUrl(const std::string& function, const std::string& symbol) {
    std::stringstream ss;
    ss << "https://www.alphavantage.co/query?function=" << function
       << "&symbol=" << symbol
       << "&apikey=" << api_key_
       << "&datatype=json";
    return ss.str();
}

std::string DataFetcher::buildPolygonUrl(const std::string& endpoint) {
    std::stringstream ss;
    ss << "https://api.polygon.io/" << endpoint
       << "?apikey=" << api_key_;
    return ss.str();
}

bool DataFetcher::fetchStockData(const std::string& symbol, const std::string& interval,
                                 int days, std::vector<OHLCVData>& data) {
    std::string url = buildAlphaVantageUrl("TIME_SERIES_DAILY_ADJUSTED", symbol);
    std::string response;
    
    if (!makeHttpRequest(url, response)) {
        return false;
    }
    
    try {
        nlohmann::json json_data = nlohmann::json::parse(response);
        
        if (json_data.contains("Error Message") || json_data.contains("Note")) {
            rag::utils::Logger::getInstance().error("API error: " + response);
            return false;
        }
        
        if (json_data.contains("Time Series (Daily)")) {
            auto time_series = json_data["Time Series (Daily)"];
            int count = 0;
            
            for (auto it = time_series.rbegin(); it != time_series.rend() && count < days; ++it, ++count) {
                OHLCVData ohlcv;
                ohlcv.timestamp = it.key();
                ohlcv.open = std::stod(it.value()["1. open"].get<std::string>());
                ohlcv.high = std::stod(it.value()["2. high"].get<std::string>());
                ohlcv.low = std::stod(it.value()["3. low"].get<std::string>());
                ohlcv.close = std::stod(it.value()["4. close"].get<std::string>());
                ohlcv.volume = std::stol(it.value()["5. volume"].get<std::string>());
                data.push_back(ohlcv);
            }
        }
        
        rag::utils::Logger::getInstance().info("Fetched " + std::to_string(data.size()) + " data points for " + symbol);
        return true;
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse JSON: " + std::string(e.what()));
        return false;
    }
}

bool DataFetcher::fetchRealTimeQuote(const std::string& symbol, double& price, double& change_percent) {
    std::string url = buildAlphaVantageUrl("GLOBAL_QUOTE", symbol);
    std::string response;
    
    if (!makeHttpRequest(url, response)) {
        return false;
    }
    
    try {
        nlohmann::json json_data = nlohmann::json::parse(response);
        
        if (json_data.contains("Global Quote")) {
            auto quote = json_data["Global Quote"];
            price = std::stod(quote["05. price"].get<std::string>());
            change_percent = std::stod(quote["10. change percent"].get<std::string>());
            return true;
        }
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse quote JSON: " + std::string(e.what()));
    }
    
    return false;
}

bool DataFetcher::fetchOptionsData(const std::string& symbol, std::vector<OptionsData>& data) {
    // Polygon.io options endpoint
    std::string url = buildPolygonUrl("v2/aggs/ticker/" + symbol + "/range/1/day/2023-01-01/2023-12-31");
    std::string response;
    
    if (!makeHttpRequest(url, response)) {
        return false;
    }
    
    // Parse options data (simplified - actual implementation would parse options chain)
    try {
        nlohmann::json json_data = nlohmann::json::parse(response);
        // Implementation depends on Polygon.io options API structure
        rag::utils::Logger::getInstance().info("Options data fetched for " + symbol);
        return true;
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse options JSON: " + std::string(e.what()));
        return false;
    }
}

bool DataFetcher::fetchNews(const std::string& symbol, int max_articles, std::vector<NewsArticle>& articles) {
    std::string url = buildAlphaVantageUrl("NEWS_SENTIMENT", symbol);
    url += "&limit=" + std::to_string(max_articles);
    std::string response;
    
    if (!makeHttpRequest(url, response)) {
        return false;
    }
    
    try {
        nlohmann::json json_data = nlohmann::json::parse(response);
        
        if (json_data.contains("feed")) {
            for (const auto& item : json_data["feed"]) {
                NewsArticle article;
                article.id = item.value("url", "");
                article.title = item.value("title", "");
                article.content = item.value("summary", "");
                article.source = item.value("source", "");
                article.published_time = item.value("time_published", "");
                
                if (item.contains("ticker_sentiment")) {
                    for (const auto& ticker : item["ticker_sentiment"]) {
                        article.tickers.push_back(ticker.value("ticker", ""));
                    }
                }
                
                articles.push_back(article);
            }
        }
        
        rag::utils::Logger::getInstance().info("Fetched " + std::to_string(articles.size()) + " news articles for " + symbol);
        return true;
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse news JSON: " + std::string(e.what()));
        return false;
    }
}

bool DataFetcher::fetchCompanyFundamentals(const std::string& symbol, nlohmann::json& fundamentals) {
    std::string url = buildAlphaVantageUrl("OVERVIEW", symbol);
    std::string response;
    
    if (!makeHttpRequest(url, response)) {
        return false;
    }
    
    try {
        fundamentals = nlohmann::json::parse(response);
        return true;
    } catch (const std::exception& e) {
        rag::utils::Logger::getInstance().error("Failed to parse fundamentals JSON: " + std::string(e.what()));
        return false;
    }
}

bool DataFetcher::fetchVolatility(const std::string& symbol, const std::string& date, double& volatility) {
    // Fetch historical data and calculate volatility
    std::vector<OHLCVData> data;
    if (!fetchStockData(symbol, "daily", 30, data)) {
        return false;
    }
    
    // Calculate realized volatility (simplified)
    if (data.size() < 2) {
        return false;
    }
    
    double sum_returns_squared = 0.0;
    for (size_t i = 1; i < data.size(); ++i) {
        double returns = (data[i].close - data[i-1].close) / data[i-1].close;
        sum_returns_squared += returns * returns;
    }
    
    volatility = std::sqrt(sum_returns_squared / (data.size() - 1)) * std::sqrt(252); // Annualized
    return true;
}

} // namespace data
} // namespace rag

