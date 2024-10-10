#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <map>
#include <mutex>
#include <cstdint>

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    void addSession(uint32_t ssrc);
    void removeSession(uint32_t ssrc);
    bool hasSession(uint32_t ssrc);

private:
    std::map<uint32_t, /* session info */> sessions_;
    std::mutex mutex_;
};

#endif // SESSION_MANAGER_H
