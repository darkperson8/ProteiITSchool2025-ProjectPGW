#include <gtest/gtest.h>
#include "bcd_utils.h"

TEST(BCDConversion, EvenLength) {
    std::string imsi = "12345678";
    auto bcd = str2bcd(imsi);
    EXPECT_EQ(bcd.size(), 4u);
    EXPECT_EQ(bcd2str(bcd.data(), bcd.size()), imsi);
}

TEST(BCDConversion, OddLength) {
    std::string imsi = "12345";
    auto bcd = str2bcd(imsi);
    EXPECT_EQ(bcd.size(), 3u);
    EXPECT_EQ(bcd2str(bcd.data(), bcd.size()), imsi);
}

TEST(BCDConversion, RoundTrip) {
    std::vector<std::string> samples = {
        "001010123456789", "9876543210", "5"
    };
    for (auto &imsi : samples) {
        auto bcd = str2bcd(imsi);
        EXPECT_EQ(bcd2str(bcd.data(), bcd.size()), imsi);
    }
}
