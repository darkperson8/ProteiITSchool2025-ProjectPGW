#ifndef SESSION_MANGER_H
#define SESSION_MANGER_H

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <functional>

// Менеджер сессий
class SessionManager {
public:
    explicit SessionManager(uint32_t timeout_sec);

    // Создать сессию, вернуть true, если успешно создалась
    bool create(const std::string& imsi);

    bool exists(const std::string& imsi);

    // Удалить все просроченные и вызвать on_expire
    void cleanup(std::function<void(const std::string&)> on_expire);

    // Graceful-удаление всех сессий с заданной скоростью
    void stop_graceful(uint32_t rate_per_sec, std::function<void(const std::string&)> on_delete);

private:
    using Clock = std::chrono::steady_clock;
    std::unordered_map<std::string, Clock::time_point> sessions_;
    uint32_t timeout_;
    std::mutex mtx_;
};

#endif //SESSION_MANGER_H
