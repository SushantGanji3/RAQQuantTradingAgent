#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace rag {
namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& filepath);
    
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string levelToString(LogLevel level);
    std::ofstream log_file_;
    LogLevel log_level_ = LogLevel::INFO;
    std::mutex mutex_;
};

} // namespace utils
} // namespace rag

