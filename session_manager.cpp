#include "session_manager.h"

SessionManager::SessionManager() {

}

SessionManager::~SessionManager() {

}

void SessionManager::addSession(uint32_t ssrc) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Add session to map if not exists
    if (sessions_.find(ssrc) == sessions_.end()) {
        sessions_[ssrc] = /* session info */;
    }
}

void SessionManager::removeSession(uint32_t ssrc) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(ssrc);
}

bool SessionManager::hasSession(uint32_t ssrc) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.find(ssrc) != sessions_.end();
}
