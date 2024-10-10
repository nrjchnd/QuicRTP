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
#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <functional>
#include <cstdint>
#include <cstddef>
#include <mutex>
#include <vector>

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
    std::mutex translatorMutex_;

    // Additional private members for RTP packet construction
    uint16_t sequenceNumber_;
    uint32_t timestamp_;
    uint32_t ssrc_;
};

#endif // TRANSLATOR_H
