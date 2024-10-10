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

int main(int argc, char* argv[]) {
    try {
        Logger::init();

        Config config;
        if (!config.loadConfig("config.conf")) {
            Logger::getLogger()->error("Failed to load configuration file");
            return -1;
        }

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

        CacheManager cacheManager(redisUri);
        SessionManager sessionManager;
        Translator translator;

        boost::asio::io_context io_context;

        int portStart = config.getInt("RTP", "port_range_start");
        int portEnd = config.getInt("RTP", "port_range_end");

        if (portStart <= 0 || portEnd <= 0 || portEnd < portStart) {
            Logger::getLogger()->error("Invalid RTP port range specified in configuration");
            return -1;
        }

        std::vector<uint16_t> availablePorts;
        for (int port = portStart; port <= portEnd; ++port) {
            availablePorts.push_back(static_cast<uint16_t>(port));
        }

        std::vector<std::shared_ptr<RtpListener>> rtpListeners;
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
                        Logger::getLogger()->warn("Received RTP packet is too short");
                    }
                });

                rtpListeners.push_back(rtpListener);
            } catch (const std::exception& e) {
                Logger::getLogger()->warn("Port {} is unavailable: {}", port, e.what());
                continue;
            }
        }

        // Initialize QUIC client
        auto quicClient = std::make_shared<QuicClient>(quicServerIp, quicServerPort);
        if (!quicClient->initialize()) {
            Logger::getLogger()->error("Failed to initialize QUIC client");
            return -1;
        }
        quicClient->start();

        translator.setRtpToQuicHandler([quicClient](const uint8_t* data, size_t len) {
            quicClient->sendData(data, len);
        });

        quicClient->setDataHandler([&](const uint8_t* data, size_t len) {
            translator.translateQuicToRtp(data, len);
        });

        translator.setQuicToRtpHandler([&](const uint8_t* data, size_t len) {
            // Implement sending data back to RTP endpoints if necessary
            // This could involve looking up the appropriate endpoint from the cache
        });

        // Run the IO context in a separate thread
        std::thread ioThread([&io_context]() {
            io_context.run();
        });

        // Keep the main thread running
        Logger::getLogger()->info("Translator is running...");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Clean up
        io_context.stop();
        ioThread.join();

    } catch (const std::exception& e) {
        Logger::getLogger()->error("Application error: {}", e.what());
        return -1;
    }

    return 0;
}
