#include "quic_client.h"
#include <iostream>

const QUIC_API_TABLE* MsQuic;

QuicClient::QuicClient(const std::string& serverIp, uint16_t serverPort)
    : serverIp_(serverIp), serverPort_(serverPort), registration_(nullptr), configuration_(nullptr), connection_(nullptr)
{
}

QuicClient::~QuicClient() {
    stop();
}

bool QuicClient::initialize() {
    QUIC_STATUS Status;
    QUIC_REGISTRATION_CONFIG RegConfig = { "quic_client", QUIC_EXECUTION_PROFILE_LOW_LATENCY };

    if (QUIC_FAILED(Status = MsQuicOpen(&MsQuic))) {
        std::cerr << "MsQuicOpen failed" << std::endl;
        return false;
    }

    if (QUIC_FAILED(Status = MsQuic->RegistrationOpen(&RegConfig, &registration_))) {
        std::cerr << "RegistrationOpen failed" << std::endl;
        return false;
    }

    // Configuration settings
    const QUIC_BUFFER Alpn = { sizeof("hq-29") - 1, (uint8_t*)"hq-29" };
    QUIC_SETTINGS Settings = {0};
    Settings.IsSet.PeerUnidiStreamCount = TRUE;
    Settings.PeerUnidiStreamCount = 100;
    Settings.IsSet.IdleTimeoutMs = TRUE;
    Settings.IdleTimeoutMs = 30000;

    if (QUIC_FAILED(Status = MsQuic->ConfigurationOpen(registration_, &Alpn, 1, &Settings, sizeof(Settings), nullptr, &configuration_))) {
        std::cerr << "ConfigurationOpen failed" << std::endl;
        return false;
    }

    if (QUIC_FAILED(Status = MsQuic->ConfigurationLoadCredential(configuration_, nullptr))) {
        std::cerr << "ConfigurationLoadCredential failed" << std::endl;
        return false;
    }

    return true;
}

void QuicClient::start() {
    QUIC_STATUS Status;

    if (QUIC_FAILED(Status = MsQuic->ConnectionOpen(registration_, ClientConnectionCallback, this, &connection_))) {
        std::cerr << "ConnectionOpen failed" << std::endl;
        return;
    }

    if (QUIC_FAILED(Status = MsQuic->ConnectionStart(connection_, configuration_, QUIC_ADDRESS_FAMILY_UNSPEC, serverIp_.c_str(), serverPort_))) {
        std::cerr << "ConnectionStart failed" << std::endl;
        MsQuic->ConnectionClose(connection_);
        connection_ = nullptr;
        return;
    }
}

void QuicClient::stop() {
    if (connection_) {
        MsQuic->ConnectionClose(connection_);
        connection_ = nullptr;
    }
    if (configuration_) {
        MsQuic->ConfigurationClose(configuration_);
        configuration_ = nullptr;
    }
    if (registration_) {
        MsQuic->RegistrationClose(registration_);
        registration_ = nullptr;
    }
    MsQuicClose(MsQuic);
}

void QuicClient::sendData(const uint8_t* data, size_t len) {
    // Open stream and send data
    HQUIC Stream = nullptr;
    if (QUIC_FAILED(MsQuic->StreamOpen(connection_, QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL, ClientStreamCallback, this, &Stream))) {
        std::cerr << "StreamOpen failed" << std::endl;
        return;
    }

    if (QUIC_FAILED(MsQuic->StreamStart(Stream, QUIC_STREAM_START_FLAG_IMMEDIATE))) {
        std::cerr << "StreamStart failed" << std::endl;
        MsQuic->StreamClose(Stream);
        return;
    }

    QUIC_BUFFER buffer;
    buffer.Length = static_cast<uint32_t>(len);
    buffer.Buffer = const_cast<uint8_t*>(data);

    if (QUIC_FAILED(MsQuic->StreamSend(Stream, &buffer, 1, QUIC_SEND_FLAG_FIN, nullptr))) {
        std::cerr << "StreamSend failed" << std::endl;
        MsQuic->StreamClose(Stream);
        return;
    }
}

void QuicClient::setDataHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    dataHandler_ = handler;
}

QUIC_STATUS QUIC_API QuicClient::ClientConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event) {
    QuicClient* client = static_cast<QuicClient*>(Context);
    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        std::cout << "QUIC connected" << std::endl;
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        std::cout << "QUIC shutdown complete" << std::endl;
        MsQuic->ConnectionClose(Connection);
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QUIC_API QuicClient::ClientStreamCallback(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event) {
    QuicClient* client = static_cast<QuicClient*>(Context);
    switch (Event->Type) {
    case QUIC_STREAM_EVENT_RECEIVE:
        if (client->dataHandler_) {
            client->dataHandler_(Event->RECEIVE.Buffers->Buffer, Event->RECEIVE.Buffers->Length);
        }
        break;
    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        MsQuic->StreamShutdown(Stream, QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL, 0);
        break;
    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        MsQuic->StreamClose(Stream);
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}
