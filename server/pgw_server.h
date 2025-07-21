#ifndef PGW_SERVER_H
#define PGW_SERVER_H

#include "config.h"
#include "session_manager.h"
#include "cdr_logger.h"
#include "logger.h"
#include <atomic>
#include <thread>
#include <memory>
#include <httplib.h>

// Класс, инкапсулирующий весь сервер PGW:
// - UDP + epoll для IMSI-запросов
// - HTTP API (/check_subscriber, /stop) через cpp-httplib
// - Сессии, CDR-логирование, graceful shutdown
class PGWServer {
public:
    explicit PGWServer(const Config& config);
    ~PGWServer();
    // Запустить сервер
    void run();

private:
    void start_http();                // Настройка и запуск HTTP-сервера
    void stop_http();                 // Остановка HTTP-сервера
    void handle_udp();                // Основной цикл UDP+epoll
    void graceful_shutdown();         // Удаление оставшихся сессий

    // Конфиг и компоненты
    Config config_;                   // Параметры из JSON
    SessionManager sessions_;         // Менеджер сессий
    CdrLogger cdr_;                   // Логгер CDR
    std::shared_ptr<spdlog::logger> log_;  // Логгер spdlog

    // Флаг «работать/закончить»
    std::atomic<bool> running_;

    // UDP + epoll
    int udp_fd_{-1};
    int epoll_fd_{-1};

    // HTTP-сервер в своём потоке
    std::unique_ptr<httplib::Server> http_server_;
    std::thread http_thread_;
};

#endif //PGW_SERVER_H
