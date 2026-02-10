#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace Logging {
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    class Logger {
    public:
        static Logger& getInstance();

        void setLogLevel(LogLevel level);
        void setLogFile(const std::string& filename);
        void enableConsoleOutput(bool enable);
        void enableFileOutput(bool enable);

        void log(LogLevel level, const std::string& message, const std::string& module = "");

        void debug(const std::string& message, const std::string& module = "");
        void info(const std::string& message, const std::string& module = "");
        void warning(const std::string& message, const std::string& module = "");
        void error(const std::string& message, const std::string& module = "");
        void critical(const std::string& message, const std::string& module = "");

        ~Logger();

    private:
        Logger();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        std::string getCurrentTimestamp();
        std::string logLevelToString(LogLevel level);
        std::string getColorCode(LogLevel level);
        void writeLog(const std::string& formattedMessage);

        LogLevel m_currentLevel;
        std::ofstream m_logFile;
        std::mutex m_mutex;
        bool m_consoleOutput;
        bool m_fileOutput;
    };
}