#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <functional>
#include <cstdint>
#include <cstddef>

class Translator {
public:
    Translator();
    ~Translator();

    void setRtpToQuicHandler(std::function<void(const uint8_t* data, size_t len)> handler);
    void setQuicToRtpHandler(std::function<void(const uint8_t* data, size_t len)> handler);

    void translateRtpToQuic(const uint8_t* data, size_t len);
    void translateQuicToRtp(const uint8_t* data, size_t len);

private:
    std::function<void(const uint8_t* data, size_t len)> rtpToQuicHandler_;
    std::function<void(const uint8_t* data, size_t len)> quicToRtpHandler_;
};

#endif // TRANSLATOR_H
