#include "session_manager.h"
#include <thread>

SessionManager::SessionManager(uint32_t timeout_sec)
    : timeout_(timeout_sec) {}

bool SessionManager::create(const std::string& imsi) {
    std::lock_guard lk(mtx_);
    if (sessions_.count(imsi)) return false;
    sessions_[imsi] = Clock::now() + std::chrono::seconds(timeout_);
    return true;
}

bool SessionManager::exists(const std::string& imsi) {
    std::lock_guard lk(mtx_);
    return sessions_.count(imsi) > 0;
}

void SessionManager::cleanup(std::function<void(const std::string&)> on_expire) {
    std::lock_guard lk(mtx_);
    auto now = Clock::now();
    for (auto it = sessions_.begin(); it != sessions_.end(); ) {
        if (it->second <= now) {
            on_expire(it->first);
            it = sessions_.erase(it);
        } else ++it;
    }
}

void SessionManager::stop_graceful(uint32_t rate_per_sec,
    std::function<void(const std::string&)> on_delete) {
    // Сбор всех IMSI
    std::vector<std::string> all;
    {
        std::lock_guard lk(mtx_);
        for (auto &p: sessions_) all.push_back(p.first);
    }
    // Удаляем с задержкой
    for (size_t i = 0; i < all.size(); ++i) {
        on_delete(all[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / rate_per_sec));
    }
}
