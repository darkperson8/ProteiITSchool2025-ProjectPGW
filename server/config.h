#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Конфигурация PGW-сервера
struct Config {
    std::string udp_ip;                // UDP‑IP
    uint16_t    udp_port;              // UDP‑порт
    uint32_t    session_timeout_sec;   // Таймаут сессии, сек
    std::string cdr_file;              // Путь к CDR‑файлу
    uint16_t    http_port;             // HTTP‑порт
    uint32_t    graceful_shutdown_rate;// Скорость удаления сессий (шт/сек)
    std::string log_file;              // Файл логов
    std::string log_level;             // Уровень логов
    std::vector<std::string> blacklist;// Чёрный список IMSI

    // Загрузка из JSON
    static Config load(const std::string& path);
};


#endif //CONFIG_H
