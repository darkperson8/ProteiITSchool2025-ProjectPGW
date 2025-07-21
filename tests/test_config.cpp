#include <gtest/gtest.h>
#include "config.h"
#include <fstream>
#include <cstdio>

// Cоздаёт временный JSON‑файл
static std::string make_temp_config() {
    const char *txt = R"({
      "udp_ip": "1.2.3.4",
      "udp_port": 1234,
      "session_timeout_sec": 42,
      "cdr_file": "cdr_test.log",
      "http_port": 5678,
      "graceful_shutdown_rate": 5,
      "log_file": "log_test.log",
      "log_level": "debug",
      "blacklist": ["A","B","C"]
    })";
    std::string name = "tmp_config.json";
    std::ofstream f(name);
    f << txt;
    f.close();
    return name;
}

TEST(ConfigLoad, AllFields) {
    auto fname = make_temp_config();
    Config c = Config::load(fname);
    EXPECT_EQ(c.udp_ip, "1.2.3.4");
    EXPECT_EQ(c.udp_port, 1234u);
    EXPECT_EQ(c.session_timeout_sec, 42u);
    EXPECT_EQ(c.cdr_file, "cdr_test.log");
    EXPECT_EQ(c.http_port, 5678u);
    EXPECT_EQ(c.graceful_shutdown_rate, 5u);
    EXPECT_EQ(c.log_file, "log_test.log");
    EXPECT_EQ(c.log_level, "debug");
    ASSERT_EQ(c.blacklist.size(), 3u);
    EXPECT_EQ(c.blacklist[0], "A");
    EXPECT_EQ(c.blacklist[1], "B");
    EXPECT_EQ(c.blacklist[2], "C");
    std::remove(fname.c_str());
}
