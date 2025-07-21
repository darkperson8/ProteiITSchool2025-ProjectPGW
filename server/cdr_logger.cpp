#include "cdr_logger.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>

CdrLogger::CdrLogger(const std::string& path)
    : file_(path) {}

void CdrLogger::record(const std::string& imsi, const std::string& action) {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::ofstream f(file_, std::ios::app);
    f << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ")
      << ',' << imsi << ',' << action << '\n';
}