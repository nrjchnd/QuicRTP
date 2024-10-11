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
#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <map>
#include <mutex>
#include <cstdint>

struct SessionInfo {
    // Add session-related members if needed
};

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    void addSession(uint32_t ssrc);
    void removeSession(uint32_t ssrc);
    bool hasSession(uint32_t ssrc);

private:
    std::map<uint32_t, SessionInfo> sessions_;
    std::mutex mutex_;
};

#endif // SESSION_MANAGER_H
