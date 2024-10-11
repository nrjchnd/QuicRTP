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
#ifndef RTP_LISTENER_H
#define RTP_LISTENER_H

#include <string>
#include <thread>
#include <functional>
#include <atomic>
#include <srtp2/srtp.h>
#include <boost/asio.hpp>
#include <memory>

class RtpListener {
public:
    RtpListener(boost::asio::io_context& io_context, bool isSrtp, const std::string& srtpKey);
    ~RtpListener();

    void start(uint16_t port);
    void stop();

    void setPacketHandler(std::function<void(const uint8_t* data, size_t len, const boost::asio::ip::udp::endpoint& sender)> handler);

private:
    void receive();
    void handleReceive(const boost::system::error_code& error, size_t bytes_transferred);

    bool isSrtp_;
    std::string srtpKey_;

    srtp_t srtpSession_;
    srtp_policy_t policy_;

    std::function<void(const uint8_t* data, size_t len, const boost::asio::ip::udp::endpoint& sender)> packetHandler_;

    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remoteEndpoint_;
    std::array<uint8_t, 2048> recvBuffer_;
};

#endif // RTP_LISTENER_H
