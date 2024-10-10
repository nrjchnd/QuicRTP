#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include <sw/redis++/redis++.h>
#include <string>

class CacheManager {
public:
    CacheManager(const std::string& redisUri);
    ~CacheManager();

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);

private:
    sw::redis::Redis* redis_;
};

#endif // CACHE_MANAGER_H
