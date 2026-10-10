#include "Logger.hpp"
#include "Exceptions.hpp"

#include <ctime>
#include <iostream>

namespace Logging {
    Logger::Logger() 
    : m_currentLevel(LogLevel::INFO)
    , m_consoleOutput(true)
    , m_fileOutput(false) {}

    Logger::~Logger() {
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
    }

    Logger& Logger::getInstance() {
        static Logger instance;
        return instance;
    }

    void Logger::setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentLevel = level;
    }

    void Logger::setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_logFile.is_open()) {
            m_logFile.close();
        }

        m_logFile.open(filename, std::ios::app) ;

        if (!m_logFile.is_open()) {
            throw Exceptions::ConfigException("Failed to open log file: " + filename);
        }
    }

    void Logger::enableConsoleOutput(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_consoleOutput = enable;
    }

    void Logger::enableFileOutput(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fileOutput = enable;
    }

    void Logger::log(LogLevel level, const std::string& message, const std::string& module) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (level < m_currentLevel) {
            return;
        }
        std::string timestamp = getCurrentTimestamp();
        std::string levelStr = logLevelToString(level);
        std::string moduleStr = module.empty() ? "" : "[" + module + "] ";

        std::stringstream fileMessage;
        fileMessage << "[" << timestamp << "]"
                    << "[" << std::setw(8) << std::left << levelStr << "] "
                    << moduleStr << message;

        std::stringstream consoleMessage;
        consoleMessage  << getColorCode(level)
                        << "[" << timestamp << "]"
                        << "[" << std::setw(8) << std::left << levelStr << "] "
                        << "\033[0m"
                        << moduleStr
                        << message;
        writeLog(consoleMessage.str());

        if (m_fileOutput && m_logFile.is_open()) {
            m_logFile << fileMessage.str() << "\n";
            m_logFile.flush();
        }

    }

    void Logger::debug(const std::string& message, const std::string& module) {
        log(LogLevel::DEBUG, message, module);
    }

    void Logger::info(const std::string& message, const std::string& module) {
        log(LogLevel::INFO, message, module);
    }

    void Logger::warning(const std::string& message, const std::string& module) {
        log(LogLevel::WARNING, message, module);
    }

    void Logger::error(const std::string& message, const std::string& module) {
        log(LogLevel::ERROR, message, module);
    }

    void Logger::critical(const std::string& message, const std::string& module) {
        log(LogLevel::CRITICAL, message, module);
    }

    void Logger::writeLog(const std::string& formattedMessage) {
        if (m_consoleOutput) {
            std::cout << formattedMessage << "\n";
        }
    }

    std::string Logger::getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm timeInfo{};
        if (localtime_r(&time, &timeInfo) == nullptr) {
            return "0000-00-00 00:00:00.000";
        }

        std::stringstream stringstream;
        stringstream << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
        stringstream << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        return stringstream.str();
    }

    std::string Logger::logLevelToString(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG:       return "DEBUG";
            case LogLevel::INFO:        return "INFO";
            case LogLevel::WARNING:     return "WARNING";
            case LogLevel::ERROR:       return "ERROR";
            case LogLevel::CRITICAL:    return "CRITICAL";
            default:                    return "UNKNOWN";
        }
    }

    std::string Logger::getColorCode(LogLevel level) {
        switch(level) {
            case LogLevel::DEBUG:       return "\033[36m";  // Cyan
            case LogLevel::INFO:        return "\033[32m";  // Green
            case LogLevel::WARNING:     return "\033[33m";  // Yellow
            case LogLevel::ERROR:       return "\033[31m";  // Red
            case LogLevel::CRITICAL:    return "\033[1;31m"; // Bold Red
            default:                    return "\033[0m";   // Reset
        }
    }
};