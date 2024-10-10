#ifndef QUIC_CLIENT_H
#define QUIC_CLIENT_H

#include <string>
#include <functional>
#include <msquic.h>
#include <memory>
#include <mutex>

class QuicClient {
public:
    QuicClient(const std::string& serverIp, uint16_t serverPort);
    ~QuicClient();

    bool initialize();
    void start();
    void stop();

    void sendData(const uint8_t* data, size_t len);

    void setDataHandler(std::function<void(const uint8_t* data, size_t len)> handler);

private:
    std::string serverIp_;
    uint16_t serverPort_;

    HQUIC registration_;
    HQUIC configuration_;
    HQUIC connection_;
    std::mutex connectionMutex_;

    std::function<void(const uint8_t* data, size_t len)> dataHandler_;

    static QUIC_STATUS QUIC_API ClientConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event);
    static QUIC_STATUS QUIC_API ClientStreamCallback(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event);
};

#endif // QUIC_CLIENT_H
