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
#include "rtp_listener.h"
#include "logger.h"
#include <iostream>
#include <stdexcept>
#include <srtp2/srtp.h>

RtpListener::RtpListener(boost::asio::io_context& io_context, bool isSrtp, const std::string& srtpKey)
    : isSrtp_(isSrtp), srtpKey_(srtpKey), socket_(io_context)
{
    try {
        if (isSrtp_) {
            if (srtp_init() != srtp_err_status_ok) {
                throw std::runtime_error("Failed to initialize SRTP");
            }
            memset(&policy_, 0, sizeof(policy_));
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_.rtp);
            srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_.rtcp);
            policy_.ssrc.type = ssrc_any_inbound;
            policy_.key = (uint8_t*)malloc(30);

            if (policy_.key == nullptr) {
                throw std::runtime_error("Failed to allocate memory for SRTP key");
            }

            // Key conversion from hex string to byte array
            if (srtpKey_.length() != 60) { // 30 bytes in hex representation
                throw std::runtime_error("Invalid SRTP key length");
            }

            for (size_t i = 0; i < srtpKey_.length(); i += 2) {
                std::string byteString = srtpKey_.substr(i, 2);
                uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
                policy_.key[i / 2] = byte;
            }

            if (srtp_create(&srtpSession_, &policy_) != srtp_err_status_ok) {
                throw std::runtime_error("Error creating SRTP session");
            }
        }
    } catch (const std::exception& e) {
        Logger::getLogger()->error("RtpListener initialization error: {}", e.what());
        throw;
    }
}

RtpListener::~RtpListener() {
    stop();
    if (isSrtp_) {
        srtp_dealloc(srtpSession_);
        srtp_shutdown();
        free(policy_.key);
    }
}

void RtpListener::start(uint16_t port) {
    try {
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::udp::v4(), port);
        socket_.open(endpoint.protocol());
        socket_.bind(endpoint);

        Logger::getLogger()->info("RTP Listener started on port {}", port);

        receive();
    } catch (const std::exception& e) {
        Logger::getLogger()->error("Error starting RTP listener on port {}: {}", port, e.what());
        throw;
    }
}

void RtpListener::stop() {
    try {
        socket_.close();
        Logger::getLogger()->info("RTP Listener stopped");
    } catch (const std::exception& e) {
        Logger::getLogger()->error("Error stopping RTP listener: {}", e.what());
    }
}

void RtpListener::setPacketHandler(std::function<void(const uint8_t* data, size_t len, const boost::asio::ip::udp::endpoint& sender)> handler) {
    packetHandler_ = handler;
}

void RtpListener::receive() {
    socket_.async_receive_from(
        boost::asio::buffer(recvBuffer_), remoteEndpoint_,
        [this](const boost::system::error_code& error, size_t bytes_transferred) {
            handleReceive(error, bytes_transferred);
        }
    );
}

void RtpListener::handleReceive(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        uint8_t* data = recvBuffer_.data();
        size_t len = bytes_transferred;

        if (isSrtp_) {
            srtp_err_status_t status = srtp_unprotect(srtpSession_, data, (int*)&len);
            if (status != srtp_err_status_ok) {
                Logger::getLogger()->error("Error decrypting SRTP packet");
                receive();
                return;
            }
        }

        if (packetHandler_) {
            packetHandler_(data, len, remoteEndpoint_);
        }

        receive();
    } else {
        Logger::getLogger()->error("Receive error: {}", error.message());
        // Attempt to restart receive if the error is recoverable
        if (error != boost::asio::error::operation_aborted) {
            receive();
        }
    }
}
