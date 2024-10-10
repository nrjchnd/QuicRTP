#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <memory>

class Logger {
public:
    static void init();
    static std::shared_ptr<spdlog::logger>& getLogger();
private:
    static std::shared_ptr<spdlog::logger> logger_;
};

#endif // LOGGER_H
