#include "pgw_server.h"
#include "bcd_utils.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>

PGWServer::PGWServer(const Config& config)
    : config_(config),
      sessions_(config.session_timeout_sec),
      cdr_(config.cdr_file),
      log_(get_logger()),
      running_(true),
      http_server_(std::make_unique<httplib::Server>())
{
    // Конструктор не запускает ничего — подготовка закончена
    log_->info("PGWServer конструируется");
}

PGWServer::~PGWServer() {
    // Убедимся, что потоки закрыты
    if (running_) {
        running_ = false;
        stop_http();
    }
}

void PGWServer::run() {
    // Запускаем HTTP API
    start_http();
    // Запускаем цикл обработки UDP
    handle_udp();
    // Graceful shutdown сессий
    graceful_shutdown();
    // Останавливаем HTTP
    stop_http();
    log_->info("PGWServer полностью остановлен");
}

// HTTP

void PGWServer::start_http() {
    // Маршрут проверки сессии
    http_server_->Get("/check_subscriber",
        [&](const httplib::Request& req, httplib::Response& res) {
            auto imsi = req.get_param_value("imsi");
            bool active = sessions_.exists(imsi);
            res.set_content(active ? "active" : "not active",
                            "text/plain");
            log_->info("HTTP /check_subscriber {} -> {}",
                       imsi, active ? "active" : "not active");
        });

    // Маршрут остановки
    http_server_->Get("/stop",
        [&](const httplib::Request&, httplib::Response& res) {
            running_ = false;  // сигнал UDP-циклу
            res.set_content("stopping", "text/plain");
            log_->warn("HTTP /stop получен, инициирован shutdown");
        });

    // Запускаем в отдельном потоке, чтобы не блокировать главный цикл
    http_thread_ = std::thread([this]() {
        log_->info("HTTP сервер запускается на порту {}", config_.http_port);
        http_server_->listen("0.0.0.0", config_.http_port);
    });
}

void PGWServer::stop_http() {
    if (http_server_) {
        // Останавливаем cpp-httplib
        http_server_->stop();
    }
    if (http_thread_.joinable()) {
        http_thread_.join();
    }
    log_->info("HTTP сервер остановлен");
}

// UDP + epoll

void PGWServer::handle_udp() {
    // Создаём UDP-сокет
    udp_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(config_.udp_port);
    bind(udp_fd_, (sockaddr*)&addr, sizeof(addr));

    // Устанавливаем non-blocking
    fcntl(udp_fd_, F_SETFL, O_NONBLOCK);

    // Создаём epoll и регистрируем udp_fd_
    epoll_fd_ = epoll_create1(0);
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = udp_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, udp_fd_, &ev);

    // Буфер событий
    const int MAX_EVENTS = 10;
    std::vector<epoll_event> events(MAX_EVENTS);

    log_->info("UDP+epoll цикл запущен на порту {}", config_.udp_port);

    // Главный цикл
    while (running_) {
        // Чистим просроченные сессии
        sessions_.cleanup([&](const std::string& imsi){
            cdr_.record(imsi, "expired");
            log_->info("Session expired: {}", imsi);
        });

        // Ждём событий на UDP (timeout 100 ms)
        int n = epoll_wait(epoll_fd_, events.data(), MAX_EVENTS, 100);
        if (n < 0) {
            log_->error("epoll_wait error: {}", strerror(errno));
            break;
        }

        // Обрабатываем каждый готовый дескриптор
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == udp_fd_) {
                uint8_t buf[64];
                sockaddr_in cli{};
                socklen_t clilen = sizeof(cli);

                // Читаем UDP-пакет
                ssize_t len = recvfrom(udp_fd_, buf, sizeof(buf), 0,
                                       (sockaddr*)&cli, &clilen);
                if (len <= 0) continue;

                // Декодируем IMSI
                std::string imsi = bcd2str(buf, len);
                log_->debug("UDP received IMSI: {}", imsi);

                // Логика черного списка и сессий
                bool is_black = std::find(
                    config_.blacklist.begin(),
                    config_.blacklist.end(),
                    imsi) != config_.blacklist.end();

                std::string reply;
                if (is_black) {
                    reply = "rejected";
                    cdr_.record(imsi, "rejected");
                    log_->warn("IMSI blacklisted: {}", imsi);
                }
                else if (sessions_.create(imsi)) {
                    reply = "created";
                    cdr_.record(imsi, "created");
                    log_->info("Session created: {}", imsi);
                } else {
                    reply = "created";
                    log_->info("Session exists: {}", imsi);
                }

                // Отправляем ответ
                sendto(udp_fd_, reply.data(), reply.size(), 0,
                       (sockaddr*)&cli, clilen);
            }
        }
    }

    // Закрываем дескрипторы epoll/UDP
    close(epoll_fd_);
    close(udp_fd_);
    log_->info("UDP+epoll цикл завершён");
}

// Graceful Shutdown

void PGWServer::graceful_shutdown() {
    log_->info("Начинаем graceful shutdown сессий");
    sessions_.stop_graceful(
        config_.graceful_shutdown_rate,
        [&](const std::string& imsi) {
            cdr_.record(imsi, "deleted");
            log_->info("Session deleted: {}", imsi);
        });
}
