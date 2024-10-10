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
#include <cstring>
#include <iostream>

Translator::Translator()
    : sequenceNumber_(0), timestamp_(0), ssrc_(0x12345678) {
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

    // Ensure the RTP packet is at least the minimum size
    if (len < 12) {
        std::cerr << "Invalid RTP packet: too short" << std::endl;
        return;
    }

    // Parse RTP header
    uint8_t version = (data[0] >> 6) & 0x03;
    uint8_t padding = (data[0] >> 5) & 0x01;
    uint8_t extension = (data[0] >> 4) & 0x01;
    uint8_t csrcCount = data[0] & 0x0F;
    uint8_t marker = (data[1] >> 7) & 0x01;
    uint8_t payloadType = data[1] & 0x7F;
    uint16_t sequenceNumber = (data[2] << 8) | data[3];
    uint32_t timestamp = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
    uint32_t ssrc = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];

    size_t headerLength = 12 + csrcCount * 4;

    if (len < headerLength) {
        std::cerr << "Invalid RTP packet: incorrect header length" << std::endl;
        return;
    }

    // Handle extension header if present
    if (extension) {
        if (len < headerLength + 4) {
            std::cerr << "Invalid RTP packet: missing extension header" << std::endl;
            return;
        }
        uint16_t extensionProfile = (data[headerLength] << 8) | data[headerLength + 1];
        uint16_t extensionLength = (data[headerLength + 2] << 8) | data[headerLength + 3];
        headerLength += 4 + extensionLength * 4;

        if (len < headerLength) {
            std::cerr << "Invalid RTP packet: incomplete extension data" << std::endl;
            return;
        }
    }

    // Calculate payload length and pointer
    size_t payloadLength = len - headerLength;
    const uint8_t* payloadData = data + headerLength;

    // Send the payload over QUIC
    if (rtpToQuicHandler_) {
        rtpToQuicHandler_(payloadData, payloadLength);
    } else {
        std::cerr << "RTP to QUIC handler is not set" << std::endl;
    }
}

void Translator::translateQuicToRtp(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(translatorMutex_);

    // Prepare an RTP packet buffer
    uint8_t rtpPacket[1500]; // Max RTP packet size

    // Construct RTP header
    uint8_t version = 2;
    uint8_t padding = 0;
    uint8_t extension = 0;
    uint8_t csrcCount = 0;
    uint8_t marker = 0;
    uint8_t payloadType = 96; // Dynamic payload type

    // Increment sequence number and timestamp for each packet
    sequenceNumber_++;
    timestamp_ += 160; // Adjust as per your media's timestamp increment

    size_t headerLength = 12;

    rtpPacket[0] = (version << 6) | (padding << 5) | (extension << 4) | csrcCount;
    rtpPacket[1] = (marker << 7) | payloadType;
    rtpPacket[2] = (sequenceNumber_ >> 8) & 0xFF;
    rtpPacket[3] = sequenceNumber_ & 0xFF;
    rtpPacket[4] = (timestamp_ >> 24) & 0xFF;
    rtpPacket[5] = (timestamp_ >> 16) & 0xFF;
    rtpPacket[6] = (timestamp_ >> 8) & 0xFF;
    rtpPacket[7] = timestamp_ & 0xFF;
    rtpPacket[8] = (ssrc_ >> 24) & 0xFF;
    rtpPacket[9] = (ssrc_ >> 16) & 0xFF;
    rtpPacket[10] = (ssrc_ >> 8) & 0xFF;
    rtpPacket[11] = ssrc_ & 0xFF;

    // Copy the QUIC data into the RTP payload
    if (len > sizeof(rtpPacket) - headerLength) {
        std::cerr << "Data too large for RTP packet" << std::endl;
        return;
    }

    std::memcpy(rtpPacket + headerLength, data, len);
    size_t rtpPacketLength = headerLength + len;

    // Send the RTP packet to the handler
    if (quicToRtpHandler_) {
        quicToRtpHandler_(rtpPacket, rtpPacketLength);
    } else {
        std::cerr << "QUIC to RTP handler is not set" << std::endl;
    }
}
