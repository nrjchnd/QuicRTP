#include "cache_manager.h"

CacheManager::CacheManager(const std::string& redisUri) {
    sw::redis::ConnectionOptions connection_options;
    connection_options.url = redisUri;

    redis_ = new sw::redis::Redis(connection_options);
}

CacheManager::~CacheManager() {
    delete redis_;
}

void CacheManager::set(const std::string& key, const std::string& value) {
    redis_->set(key, value);
}

std::string CacheManager::get(const std::string& key) {
    auto val = redis_->get(key);
    if (val)
        return *val;
    else
        return "";
}
