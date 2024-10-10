#include "translator.h"

Translator::Translator() {

}

Translator::~Translator() {

}

void Translator::setRtpToQuicHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    rtpToQuicHandler_ = handler;
}

void Translator::setQuicToRtpHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    quicToRtpHandler_ = handler;
}

void Translator::translateRtpToQuic(const uint8_t* data, size_t len) {
    // For simplicity, we can directly pass the data to QUIC
    if (rtpToQuicHandler_) {
        rtpToQuicHandler_(data, len);
    }
}

void Translator::translateQuicToRtp(const uint8_t* data, size_t len) {
    // For simplicity, we can directly pass the data to RTP
    if (quicToRtpHandler_) {
        quicToRtpHandler_(data, len);
    }
}
