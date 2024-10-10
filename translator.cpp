#include "translator.h"

Translator::Translator() {
}

Translator::~Translator() {
}

void Translator::setRtpToQuicHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    std::lock_guard<std::mutex> lock(translatorMutex_);
    rtpToQuicHandler_ = handler;
}

void Translator::setQuicToRtpHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    std::lock_guard<std::mutex> lock(translatorMutex_);
    quicToRtpHandler_ = handler;
}

void Translator::translateRtpToQuic(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(translatorMutex_);
    // Perform any necessary translation or processing
    if (rtpToQuicHandler_) {
        rtpToQuicHandler_(data, len);
    }
}

void Translator::translateQuicToRtp(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(translatorMutex_);
    // Perform any necessary translation or processing
    if (quicToRtpHandler_) {
        quicToRtpHandler_(data, len);
    }
}
