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
 * distributed under an **"AS IS" BASIS,**
 * **WITHOUT WARRANTIES OR CONDITIONS OR CONDITIONS OF ANY KIND, either express or implied.**
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config.h"
#include "rtp_listener.h"
#include "quic_client.h"
#include "translator.h"
#include "session_manager.h"
#include "cache_manager.h"
#include "logger.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <functional>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <csignal>
#include <atomic>
#include <chrono>

std::atomic<bool> running(true);

void signal_handler(int) {
    running = false;
}

int main(int argc, char* argv[]) {
    try {
        // Initialize the logger
        Logger::init();

        // Load configuration
        Config config;
        if (!config.loadConfig()) {
            Logger::getLogger()->error("Failed to load configuration file");
            return -1;
        }

        // Retrieve configuration settings
        bool isSrtp = config.getBool("SRTP", "enable");
        std::string srtpKey;

        // Securely load SRTP key from environment variable or secure storage
        const char* srtpKeyEnv = std::getenv("SRTP_KEY");
        if (isSrtp) {
            if (srtpKeyEnv == nullptr) {
                Logger::getLogger()->error("SRTP is enabled but SRTP_KEY environment variable is not set");
                return -1;
            }
            srtpKey = srtpKeyEnv;
            if (srtpKey.length() != 60) { // 30 bytes in hex representation
                Logger::getLogger()->error("Invalid SRTP key length");
                return -1;
            }
        }

        std::string redisUri = config.get("Cache", "redis_uri");
        std::string logLevel = config.get("Logging", "level");
        std::string quicServerIp = config.get("QUIC", "server_ip");
        int quicServerPort = config.getInt("QUIC", "server_port");

        // Set log level
        if (!logLevel.empty()) {
            if (logLevel == "debug") {
                spdlog::set_level(spdlog::level::debug);
            } else if (logLevel == "info") {
                spdlog::set_level(spdlog::level::info);
            } else if (logLevel == "warn") {
                spdlog::set_level(spdlog::level::warn);
            } else if (logLevel == "error") {
                spdlog::set_level(spdlog::level::err);
            }
        }

        // Initialize components
        CacheManager cacheManager(redisUri);
        SessionManager sessionManager;
        Translator translator;

        boost::asio::io_context io_context;

        // Retrieve RTP port range from configuration
        int portStart = config.getInt("RTP", "port_range_start");
        int portEnd = config.getInt("RTP", "port_range_end");

        if (portStart <= 0 || portEnd <= 0 || portEnd < portStart) {
            Logger::getLogger()->error("Invalid RTP port range specified in configuration");
            return -1;
        }

        // Prepare list of available ports
        std::vector<uint16_t> availablePorts;
        for (int port = portStart; port <= portEnd; ++port) {
            availablePorts.push_back(static_cast<uint16_t>(port));
        }

        // Vector to hold RTP listener objects
        std::vector<std::shared_ptr<RtpListener>> rtpListeners;

        // Initialize RTP listeners
        for (uint16_t port : availablePorts) {
            try {
                auto rtpListener = std::make_shared<RtpListener>(io_context, isSrtp, srtpKey);
                rtpListener->start(port);

                // Set packet handler
                rtpListener->setPacketHandler([&](const uint8_t* data, size_t len, const boost::asio::ip::udp::endpoint& sender) {
                    // Extract SSRC from RTP header
                    if (len >= 12) {
                        uint32_t ssrc = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];

                        // Cache the mapping between SSRC and sender endpoint
                        cacheManager.set(std::to_string(ssrc), sender.address().to_string() + ":" + std::to_string(sender.port()));

                        // Session management
                        sessionManager.addSession(ssrc);

                        // Translation
                        translator.translateRtpToQuic(data, len);
                    } else {
                        Logger::getLogger()->warn("Received RTP packet is too short from {}:{}", sender.address().to_string(), sender.port());
                    }
                });

                rtpListeners.push_back(rtpListener);
            } catch (const std::exception& e) {
                Logger::getLogger()->warn("Port {} is unavailable: {}", port, e.what());
                continue;
            }
        }

        if (rtpListeners.empty()) {
            Logger::getLogger()->error("No RTP listeners could be started. Exiting.");
            return -1;
        }

        // Initialize QUIC client
        auto quicClient = std::make_shared<QuicClient>(quicServerIp, static_cast<uint16_t>(quicServerPort));
        if (!quicClient->initialize()) {
            Logger::getLogger()->error("Failed to initialize QUIC client");
            return -1;
        }
        quicClient->start();

        // Set up the translator handlers
        translator.setRtpToQuicHandler([quicClient](const uint8_t* data, size_t len) {
            quicClient->sendData(data, len);
        });

        quicClient->setDataHandler([&](const uint8_t* data, size_t len) {
            translator.translateQuicToRtp(data, len);
        });

        translator.setQuicToRtpHandler([&](const uint8_t* data, size_t len) {
            // Implement sending data back to RTP endpoints if necessary
            // Retrieve SSRC from RTP header to find the destination
            if (len >= 12) {
                uint32_t ssrc = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];

                // Retrieve the endpoint from cache
                std::string endpointStr = cacheManager.get(std::to_string(ssrc));
                if (!endpointStr.empty()) {
                    // Parse endpoint string to get IP and port
                    size_t colonPos = endpointStr.find(':');
                    if (colonPos != std::string::npos) {
                        std::string ipStr = endpointStr.substr(0, colonPos);
                        uint16_t port = static_cast<uint16_t>(std::stoi(endpointStr.substr(colonPos + 1)));

                        // Create a socket and send the RTP packet
                        boost::asio::ip::udp::socket socket(io_context);
                        boost::asio::ip::udp::endpoint destination(boost::asio::ip::address::from_string(ipStr), port);

                        socket.open(boost::asio::ip::udp::v4());
                        socket.send_to(boost::asio::buffer(data, len), destination);
                        socket.close();

                        Logger::getLogger()->debug("Sent RTP packet to {}:{}", ipStr, port);
                    } else {
                        Logger::getLogger()->warn("Invalid endpoint format for SSRC {}", ssrc);
                    }
                } else {
                    Logger::getLogger()->warn("No endpoint found for SSRC {}", ssrc);
                }
            } else {
                Logger::getLogger()->warn("Received RTP packet is too short for sending back");
            }
        });

        // Run the IO context in a separate thread
        std::thread ioThread([&io_context]() {
            try {
                io_context.run();
            } catch (const std::exception& e) {
                Logger::getLogger()->error("IO context error: {}", e.what());
            }
        });

        // Signal handling for graceful shutdown
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        // Keep the main thread running
        Logger::getLogger()->info("Translator is running...");
        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Clean up
        Logger::getLogger()->info("Shutting down...");

        io_context.stop();
        if (ioThread.joinable()) {
            ioThread.join();
        }

        quicClient->stop();

        for (auto& listener : rtpListeners) {
            listener->stop();
        }

    } catch (const std::exception& e) {
        Logger::getLogger()->error("Application error: {}", e.what());
        return -1;
    }

    return 0;
}
