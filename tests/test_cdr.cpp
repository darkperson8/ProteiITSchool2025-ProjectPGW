#include <gtest/gtest.h>
#include "cdr_logger.h"
#include <fstream>
#include <cstdio>
#include <regex>

TEST(CdrLogger, RecordFormat) {
    const char* fname = "test_cdr.log";
    std::remove(fname);

    CdrLogger cdr(fname);
    cdr.record("XYZ", "created");
    cdr.record("ABC", "expired");

    std::ifstream f(fname);
    ASSERT_TRUE(f.is_open());
    std::string line1, line2;
    std::getline(f, line1);
    std::getline(f, line2);
    f.close();

    // Проверяем формат TIMESTAMP,IMSI,ACTION
    std::regex re(R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z,(XYZ|ABC),(created|expired)$)");
    EXPECT_TRUE(std::regex_match(line1, re));
    EXPECT_TRUE(std::regex_match(line2, re));

    std::remove(fname);
}
