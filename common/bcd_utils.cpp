#include "bcd_utils.h"
#include <cstdint>
#include <string>
#include <vector>

std::vector<uint8_t> str2bcd(const std::string& s) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i < s.size(); i += 2) {
        uint8_t lo = s[i] - '0';
        uint8_t hi = (i + 1 < s.size()) ? s[i + 1] - '0' : 0x0F;
        out.push_back((hi << 4) | lo);
    }
    return out;
}

std::string bcd2str(const uint8_t* buf, size_t len) {
    std::string s;
    s.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        uint8_t lo = buf[i] & 0x0F;
        uint8_t hi = buf[i] >> 4;
        s.push_back(char('0' + lo));
        if (hi != 0x0F) s.push_back(char('0' + hi));
    }
    return s;
}
