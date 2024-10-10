#include "logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

void Logger::init() {
    logger_ = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    logger_->set_level(spdlog::level::info);
}

std::shared_ptr<spdlog::logger>& Logger::getLogger() {
    return logger_;
}
