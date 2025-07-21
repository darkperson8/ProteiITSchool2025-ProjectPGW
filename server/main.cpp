#include "pgw_server.h"
#include "config.h"
#include "logger.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: pgw_server <config.json>\n";
        return 1;
    }
    // Загружаем конфигурацию
    Config cfg = Config::load(argv[1]);
    // Инициализируем логгер
    init_logger(cfg.log_file, cfg.log_level);
    // Создаём и запускаем сервер
    PGWServer server(cfg);
    server.run();
    return 0;
}
