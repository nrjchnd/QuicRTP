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
 * distributed under an **"AS IS" BASIS,**
 * **WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.**
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "session_manager.h"
#include <stdexcept>

SessionManager::SessionManager() {
    // Constructor implementation
}

SessionManager::~SessionManager() {
    // Destructor implementation
}

void SessionManager::addSession(uint32_t ssrc) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_[ssrc] = SessionInfo();
}

void SessionManager::removeSession(uint32_t ssrc) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(ssrc);
}

bool SessionManager::hasSession(uint32_t ssrc) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.find(ssrc) != sessions_.end();
}
