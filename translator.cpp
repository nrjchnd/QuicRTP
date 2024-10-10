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
 * distributed under the License is distributed on an **"AS IS" BASIS,**
 * **WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.**
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
