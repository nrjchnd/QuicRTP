#include "config.h"
#include "rtp_listener.h"
#include "quic_client.h"
#include "translator.h"
#include "session_manager.h"
#include "cache_manager.h"
#include <iostream>
#include <thread>
#include <functional>
#include <vector>
#include <unordered_map>

int main(int argc, char* argv[]) {
    Config config;
    if (!config.loadConfig("config.conf")) {
        std::cerr << "Failed to load configuration file" << std::endl;
        return -1;
    }

    bool isSrtp = config.getBool("SRTP", "enable");
    std::string srtpKey = config.get("SRTP", "key");
    std::string redisUri = config.get("Cache", "redis_uri");

    CacheManager cacheManager(redisUri);
    SessionManager sessionManager;
    Translator translator;

    // Parse endpoints from configuration
    std::vector<std::thread> threads;
    std::unordered_map<std::string, RtpListener*> rtpListeners;
    std::unordered_map<std::string, QuicClient*> quicClients;

    for (int i = 1; ; ++i) {
        std::string endpoint = "endpoint" + std::to_string(i) + "_rtp_ip";
        std::string rtpIp = config.get("Endpoints", endpoint);
        if (rtpIp.empty())
            break; // No more endpoints

        std::string rtpPortKey = "endpoint" + std::to_string(i) + "_rtp_port";
        int rtpPort = config.getInt("Endpoints", rtpPortKey);

        std::string quicIpKey = "endpoint" + std::to_string(i) + "_quic_ip";
        std::string quicIp = config.get("Endpoints", quicIpKey);

        std::string quicPortKey = "endpoint" + std::to_string(i) + "_quic_port";
        int quicPort = config.getInt("Endpoints", quicPortKey);

        // Initialize RTP listener
        RtpListener* rtpListener = new RtpListener(rtpIp, rtpPort, isSrtp, srtpKey);
        rtpListeners[rtpIp + ":" + std::to_string(rtpPort)] = rtpListener;

        // Initialize QUIC client
        QuicClient* quicClient = new QuicClient(quicIp, quicPort);
        if (!quicClient->initialize()) {
            std::cerr << "Failed to initialize QUIC client for endpoint " << i << std::endl;
            continue;
        }
        quicClient->start();
        quicClients[quicIp + ":" + std::to_string(quicPort)] = quicClient;

        // Set up handlers
        rtpListener->setPacketHandler([&](const uint8_t* data, size_t len) {
            // Extract SSRC from RTP header
            uint32_t ssrc = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];

            // Cache the mapping
            cacheManager.set(std::to_string(ssrc), quicIp + ":" + std::to_string(quicPort));

            // Session management
            sessionManager.addSession(ssrc);

            // Translation
            translator.translateRtpToQuic(data, len);
        });

        translator.setRtpToQuicHandler([=](const uint8_t* data, size_t len) {
            quicClient->sendData(data, len);
        });

        quicClient->setDataHandler([&](const uint8_t* data, size_t len) {
            translator.translateQuicToRtp(data, len);
        });

        translator.setQuicToRtpHandler([=](const uint8_t* data, size_t len) {
            // Implement RTP sending if needed
        });

        // Start RTP listener
        rtpListener->start();
    }

    // Keep the main thread running
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up
    for (auto& pair : rtpListeners) {
        pair.second->stop();
        delete pair.second;
    }
    for (auto& pair : quicClients) {
        pair.second->stop();
        delete pair.second;
    }

    return 0;
}
