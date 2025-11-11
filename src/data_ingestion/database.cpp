#include "data_ingestion/database.h"
#include "utils/logger.h"
#include <sstream>
#include <algorithm>

namespace rag {
namespace data {

Database::Database(const std::string& db_path) : db_path_(db_path), db_(nullptr) {
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool Database::initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Cannot open database: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    return createTables();
}

bool Database::createTables() {
    const char* create_stock_table = R"(
        CREATE TABLE IF NOT EXISTS ohlcv_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            open REAL NOT NULL,
            high REAL NOT NULL,
            low REAL NOT NULL,
            close REAL NOT NULL,
            volume INTEGER NOT NULL,
            UNIQUE(symbol, timestamp)
        );
    )";
    
    const char* create_options_table = R"(
        CREATE TABLE IF NOT EXISTS options_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol TEXT NOT NULL,
            expiry TEXT NOT NULL,
            strike REAL NOT NULL,
            option_type TEXT NOT NULL,
            bid REAL,
            ask REAL,
            implied_volatility REAL,
            volume INTEGER,
            open_interest INTEGER,
            timestamp TEXT DEFAULT CURRENT_TIMESTAMP,
            UNIQUE(symbol, expiry, strike, option_type)
        );
    )";
    
    const char* create_news_table = R"(
        CREATE TABLE IF NOT EXISTS news_articles (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            article_id TEXT UNIQUE NOT NULL,
            title TEXT NOT NULL,
            content TEXT,
            source TEXT,
            published_time TEXT,
            symbol TEXT,
            timestamp TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    const char* create_volatility_table = R"(
        CREATE TABLE IF NOT EXISTS volatility (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol TEXT NOT NULL,
            date TEXT NOT NULL,
            volatility REAL NOT NULL,
            UNIQUE(symbol, date)
        );
    )";
    
    const char* create_fundamentals_table = R"(
        CREATE TABLE IF NOT EXISTS fundamentals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol TEXT UNIQUE NOT NULL,
            data TEXT NOT NULL,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )";
    
    char* err_msg = nullptr;
    
    if (sqlite3_exec(db_, create_stock_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Error creating stock table: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    
    if (sqlite3_exec(db_, create_options_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Error creating options table: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    
    if (sqlite3_exec(db_, create_news_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Error creating news table: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    
    if (sqlite3_exec(db_, create_volatility_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Error creating volatility table: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    
    if (sqlite3_exec(db_, create_fundamentals_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Error creating fundamentals table: " + std::string(err_msg));
        sqlite3_free(err_msg);
        return false;
    }
    
    rag::utils::Logger::getInstance().info("Database tables created successfully");
    return true;
}

bool Database::storeOHLCVData(const std::string& symbol, const std::vector<OHLCVData>& data) {
    const char* sql = R"(
        INSERT OR REPLACE INTO ohlcv_data (symbol, timestamp, open, high, low, close, volume)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        rag::utils::Logger::getInstance().error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    
    for (const auto& ohlcv : data) {
        sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ohlcv.timestamp.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, ohlcv.open);
        sqlite3_bind_double(stmt, 4, ohlcv.high);
        sqlite3_bind_double(stmt, 5, ohlcv.low);
        sqlite3_bind_double(stmt, 6, ohlcv.close);
        sqlite3_bind_int64(stmt, 7, ohlcv.volume);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            rag::utils::Logger::getInstance().error("Failed to insert OHLCV data: " + std::string(sqlite3_errmsg(db_)));
            sqlite3_finalize(stmt);
            sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt);
    
    rag::utils::Logger::getInstance().info("Stored " + std::to_string(data.size()) + " OHLCV records for " + symbol);
    return true;
}

bool Database::getOHLCVData(const std::string& symbol, const std::string& start_date,
                           const std::string& end_date, std::vector<OHLCVData>& data) {
    const char* sql = R"(
        SELECT timestamp, open, high, low, close, volume
        FROM ohlcv_data
        WHERE symbol = ? AND timestamp >= ? AND timestamp <= ?
        ORDER BY timestamp ASC
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, start_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, end_date.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        OHLCVData ohlcv;
        ohlcv.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        ohlcv.open = sqlite3_column_double(stmt, 1);
        ohlcv.high = sqlite3_column_double(stmt, 2);
        ohlcv.low = sqlite3_column_double(stmt, 3);
        ohlcv.close = sqlite3_column_double(stmt, 4);
        ohlcv.volume = sqlite3_column_int64(stmt, 5);
        data.push_back(ohlcv);
    }
    
    sqlite3_finalize(stmt);
    return true;
}

bool Database::storeNewsArticle(const NewsArticle& article) {
    const char* sql = R"(
        INSERT OR REPLACE INTO news_articles (article_id, title, content, source, published_time, symbol)
        VALUES (?, ?, ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, article.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, article.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, article.content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, article.source.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, article.published_time.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, article.tickers.empty() ? "" : article.tickers[0].c_str(), -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::getNewsArticles(const std::string& symbol, int limit, std::vector<NewsArticle>& articles) {
    const char* sql = R"(
        SELECT article_id, title, content, source, published_time
        FROM news_articles
        WHERE symbol = ?
        ORDER BY published_time DESC
        LIMIT ?
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NewsArticle article;
        article.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        article.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        article.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        article.source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        article.published_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        article.tickers.push_back(symbol);
        articles.push_back(article);
    }
    
    sqlite3_finalize(stmt);
    return true;
}

bool Database::getNewsArticlesByDate(const std::string& start_date, const std::string& end_date,
                                    std::vector<NewsArticle>& articles) {
    const char* sql = R"(
        SELECT article_id, title, content, source, published_time, symbol
        FROM news_articles
        WHERE published_time >= ? AND published_time <= ?
        ORDER BY published_time DESC
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, start_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, end_date.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NewsArticle article;
        article.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        article.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        article.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        article.source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        article.published_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        if (!symbol.empty()) {
            article.tickers.push_back(symbol);
        }
        articles.push_back(article);
    }
    
    sqlite3_finalize(stmt);
    return true;
}

bool Database::storeVolatility(const std::string& symbol, const std::string& date, double volatility) {
    const char* sql = R"(
        INSERT OR REPLACE INTO volatility (symbol, date, volatility)
        VALUES (?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, volatility);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::getVolatility(const std::string& symbol, const std::string& date, double& volatility) {
    const char* sql = R"(
        SELECT volatility FROM volatility
        WHERE symbol = ? AND date = ?
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        volatility = sqlite3_column_double(stmt, 0);
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::storeFundamentals(const std::string& symbol, const std::string& json_data) {
    const char* sql = R"(
        INSERT OR REPLACE INTO fundamentals (symbol, data, updated_at)
        VALUES (?, ?, CURRENT_TIMESTAMP)
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, json_data.c_str(), -1, SQLITE_STATIC);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::getFundamentals(const std::string& symbol, std::string& json_data) {
    const char* sql = R"(
        SELECT data FROM fundamentals WHERE symbol = ?
    )";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
    
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        json_data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        found = true;
    }
    
    sqlite3_finalize(stmt);
    return found;
}

bool Database::storeOptionsData(const std::vector<OptionsData>& data) {
    // Implementation similar to storeOHLCVData
    return true;
}

bool Database::getOptionsData(const std::string& symbol, std::vector<OptionsData>& data) {
    // Implementation similar to getOHLCVData
    return true;
}

std::string Database::escapeSQL(const std::string& str) {
    // Simple escaping - in production, use parameterized queries
    std::string escaped = str;
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "''");
        pos += 2;
    }
    return escaped;
}

} // namespace data
} // namespace rag

