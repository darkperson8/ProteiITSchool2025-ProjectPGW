#include "logger.h"
#include <algorithm>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

static std::shared_ptr<spdlog::logger> logger_;

void init_logger(const std::string& file, const std::string& level) {
    std::string lvl_str = level;
    std::transform(lvl_str.begin(), lvl_str.end(), lvl_str.begin(),
        [](unsigned char c) { return std::tolower(c); });

    auto lvl = spdlog::level::from_str(lvl_str);
    auto logger = spdlog::get("pgw_logger");
    if (!logger) {
        logger = spdlog::basic_logger_mt("pgw_logger", file);
    }
    logger->set_level(lvl);
    logger->flush_on(lvl);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    spdlog::set_default_logger(logger);
    logger_ = logger;
}

std::shared_ptr<spdlog::logger> get_logger() {
    return logger_;
}