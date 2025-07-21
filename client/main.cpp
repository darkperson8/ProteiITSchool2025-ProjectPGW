#include "logger.h"
#include "bcd_utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

// Преобразование бинарных данных в hex-строку
std::string to_hex_string(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]) << " ";
    }
    return oss.str();
}


int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <config.json> <IMSI>\n";
        return 1;
    }

    // Чтение конфигурации
    json cfg;
    std::ifstream f(argv[1]);
    if (!f.is_open()) {
        std::cerr << "Ошибка открытия файла: " << argv[1] << "\n";
        return 1;
    }
    try {
        f >> cfg;
    } catch (const json::exception& e) {
        std::cerr << "Ошибка парсинга JSON: " << e.what() << "\n";
        return 1;
    }

    // Извлечение параметров конфигурации
    std::string ip, log_file, log_level;
    int port;
    try {
        ip = cfg["server_ip"].get<std::string>();
        port = cfg["server_port"].get<int>();
        log_file = cfg["log_file"].get<std::string>();
        log_level = cfg["log_level"].get<std::string>();
    } catch (const json::exception& e) {
        std::cerr << "Ошибка чтения конфигурации: " << e.what() << "\n";
        return 1;
    }
    std::string imsi = argv[2];

    // Инициализация логгера
    init_logger(log_file, log_level);
    auto logger = get_logger();
    logger->info("Запуск клиента. IMSI: {}", imsi);

    // Преобразование IMSI в BCD
    auto buf = str2bcd(imsi);
    logger->debug("IMSI в BCD: {}", to_hex_string(buf.data(), buf.size()));

    // Создание сокета
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        logger->error("Ошибка создания сокета: {}", strerror(errno));
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv.sin_addr) <= 0) {
        logger->error("Ошибка преобразования IP: {}", strerror(errno));
        close(sock);
        return 1;
    }

    // Отправка данных
    ssize_t sent = sendto(sock, buf.data(), buf.size(), 0, (sockaddr*)&serv, sizeof(serv));
    if (sent < 0) {
        logger->error("Ошибка отправки: {}", strerror(errno));
        close(sock);
        return 1;
    }
    logger->info("Отправлено {} байт на {}:{}", sent, ip, port);

    // Получение ответа
    char resp[32] = {};
    socklen_t slen = sizeof(serv);
    ssize_t recv_len = recvfrom(sock, resp, sizeof(resp) - 1, 0, (sockaddr*)&serv, &slen);
    if (recv_len < 0) {
        logger->error("Ошибка получения ответа: {}", strerror(errno));
    } else {
        resp[recv_len] = '\0';
        logger->info("Ответ сервера: {}", resp);
        std::cout << "Ответ: " << resp << "\n";
    }

    close(sock);
    logger->info("Клиент завершен.");
    return 0;
}