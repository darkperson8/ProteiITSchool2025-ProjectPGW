#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string>
#include <spdlog/logger.h>

// Инициализация логгера
void init_logger(const std::string& file, const std::string& level);
// Глобальный доступ
std::shared_ptr<spdlog::logger> get_logger();

#endif //LOGGER_H
