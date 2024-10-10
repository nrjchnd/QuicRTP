#include "rtp_listener.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

RtpListener::RtpListener(const std::string& ip, int port, bool isSrtp, const std::string& srtpKey)
    : ip_(ip), port_(port), isSrtp_(isSrtp), srtpKey_(srtpKey), sockfd_(-1), running_(false), srtpSession_(nullptr)
{
    // Initialize SRTP if needed
    if (isSrtp_) {
        srtp_init();
        memset(&policy_, 0, sizeof(policy_));

        // Assuming AES_CM_128_HMAC_SHA1_80 for simplicity
        srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_.rtp);
        srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy_.rtcp);
        policy_.ssrc.type = ssrc_any_inbound;
        policy_.key = (uint8_t*)malloc(30); // Key length for AES_CM_128_HMAC_SHA1_80 is 30 bytes

        // Key conversion from hex string to byte array
        for (size_t i = 0; i < srtpKey_.length() && i / 2 < 30; i += 2) {
            std::string byteString = srtpKey_.substr(i, 2);
            uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            policy_.key[i / 2] = byte;
        }

        srtp_err_status_t status = srtp_create(&srtpSession_, &policy_);
        if (status != srtp_err_status_ok) {
            std::cerr << "Error creating SRTP session" << std::endl;
        }
    }
}

RtpListener::~RtpListener() {
    stop();
    if (isSrtp_) {
        srtp_dealloc(srtpSession_);
        srtp_shutdown();
    }
}

void RtpListener::start() {
    running_ = true;
    listenerThread_ = std::thread(&RtpListener::listenLoop, this);
}

void RtpListener::stop() {
    running_ = false;
    if (listenerThread_.joinable())
        listenerThread_.join();

    if (sockfd_ != -1)
        close(sockfd_);
}

void RtpListener::setPacketHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    packetHandler_ = handler;
}

void RtpListener::listenLoop() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr(ip_.c_str());

    if (bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return;
    }

    uint8_t buffer[1500];

    while (running_) {
        ssize_t len = recvfrom(sockfd_, buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (len > 0) {
            if (isSrtp_) {
                srtp_err_status_t status = srtp_unprotect(srtpSession_, buffer, (int*)&len);
                if (status != srtp_err_status_ok) {
                    std::cerr << "Error decrypting SRTP packet" << std::endl;
                    continue;
                }
            }

            if (packetHandler_) {
                packetHandler_(buffer, len);
            }
        }
    }
}
