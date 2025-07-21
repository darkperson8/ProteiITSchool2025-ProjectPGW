#ifndef CDR_LOGGER_H
#define CDR_LOGGER_H

#include <string>

// Логирование CDR
class CdrLogger {
public:
    explicit CdrLogger(const std::string& path);
    void record(const std::string& imsi, const std::string& action);
private:
    std::string file_;
};

#endif //CDR_LOGGER_H
