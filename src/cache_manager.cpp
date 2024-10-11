/*
 * Copyright 2024 nrjchnd@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed on an **"AS IS" BASIS,**
 * **WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.**
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cache_manager.h"
#include <regex>
#include <stdexcept>

CacheManager::CacheManager(const std::string& redisUri) {
    sw::redis::ConnectionOptions connection_options;

    // Parse redisUri to extract host and port
    // Assuming redisUri is in the format "tcp://host:port"
    std::regex uri_regex(R"(^(?:tcp://)?([^:]+)(?::(\d+))?$)");
    std::smatch matches;
    if (std::regex_match(redisUri, matches, uri_regex)) {
        connection_options.host = matches[1];
        if (matches[2].matched) {
            connection_options.port = static_cast<int>(std::stoi(matches[2]));
        } else {
            connection_options.port = 6379; // Default Redis port
        }
    } else {
        throw std::invalid_argument("Invalid Redis URI format");
    }

    // Initialize the Redis client
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
