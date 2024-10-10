#ifndef RTP_LISTENER_H
#define RTP_LISTENER_H

#include <string>
#include <thread>
#include <functional>
#include <atomic>
#include <srtp2/srtp.h>

class RtpListener {
public:
    RtpListener(const std::string& ip, int port, bool isSrtp, const std::string& srtpKey);
    ~RtpListener();

    void start();
    void stop();

    void setPacketHandler(std::function<void(const uint8_t* data, size_t len)> handler);

private:
    void listenLoop();

    std::string ip_;
    int port_;
    bool isSrtp_;
    std::string srtpKey_;

    int sockfd_;
    std::thread listenerThread_;
    std::atomic<bool> running_;

    srtp_t srtpSession_;
    srtp_policy_t policy_;

    std::function<void(const uint8_t* data, size_t len)> packetHandler_;
};

#endif // RTP_LISTENER_H
