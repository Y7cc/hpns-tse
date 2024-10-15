#ifndef TSE_LOGGER
#define TSE_LOGGER

#include <map>
#include <string>
#include <vector>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"


class Logger {
public:
    static std::shared_ptr<spdlog::logger> getLogger(const std::string& loggerName);
    static void shutdown(void);
    static void setLogFilename(std::string name);

private:
    static std::shared_ptr<spdlog::logger> createLogger(const std::string& loggerName);
    static void initSinks();
    static std::vector<spdlog::sink_ptr> sinks;
    static std::vector<std::shared_ptr<spdlog::logger>> loggers;

    static std::string log_filename;

};

#endif