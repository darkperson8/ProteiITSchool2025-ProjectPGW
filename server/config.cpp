#include "config.h"
#include <fstream>
using json = nlohmann::json;

Config Config::load(const std::string& path) {
    std::ifstream f(path);
    json j; f >> j;

    Config c;
    c.udp_ip                 = j.at("udp_ip");
    c.udp_port               = j.at("udp_port");
    c.session_timeout_sec    = j.at("session_timeout_sec");
    c.cdr_file               = j.at("cdr_file");
    c.http_port              = j.at("http_port");
    c.graceful_shutdown_rate = j.at("graceful_shutdown_rate");
    c.log_file               = j.at("log_file");
    c.log_level              = j.at("log_level");
    c.blacklist              = j.at("blacklist").get<std::vector<std::string>>();
    return c;
}
