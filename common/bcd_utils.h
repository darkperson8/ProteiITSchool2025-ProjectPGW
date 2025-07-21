#ifndef BCD_UTILS_H
#define BCD_UTILS_H
#include <cstdint>
#include <string>
#include <vector>

// Кодирование строки в BCD формат
std::vector<uint8_t> str2bcd(const std::string& s);

// Преобразование BCD-буфера в строку IMSI
std::string bcd2str(const uint8_t* buf, size_t len);

#endif //BCD_UTILS_H
