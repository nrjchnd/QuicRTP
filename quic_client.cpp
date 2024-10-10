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
#include "quic_client.h"
#include "logger.h"
#include <iostream>
#include <stdexcept>

const QUIC_API_TABLE* MsQuic;
HQUIC registration_ = nullptr;

QuicClient::QuicClient(const std::string& serverIp, uint16_t serverPort)
    : serverIp_(serverIp), serverPort_(serverPort), configuration_(nullptr), connection_(nullptr)
{
    // Initialize MsQuic
    if (QUIC_FAILED(MsQuicOpen(&MsQuic))) {
        throw std::runtime_error("MsQuicOpen failed");
    }

    // Create a registration for the app
    QUIC_STATUS status = MsQuic->RegistrationOpen(nullptr, &registration_);
    if (QUIC_FAILED(status)) {
        throw std::runtime_error("RegistrationOpen failed");
    }
}

QuicClient::~QuicClient() {
    stop();
    if (registration_) {
        MsQuic->RegistrationClose(registration_);
        registration_ = nullptr;
    }
    MsQuicClose(MsQuic);
}

bool QuicClient::initialize() {
    QUIC_STATUS status;

    // Configuration settings
    const char* alpn = "hq-29";
    QUIC_BUFFER alpnBuffer;
    alpnBuffer.Length = (uint32_t)strlen(alpn);
    alpnBuffer.Buffer = (uint8_t*)alpn;

    QUIC_SETTINGS settings = {0};
    settings.IsSet.PeerUnidiStreamCount = TRUE;
    settings.PeerUnidiStreamCount = 100;
    settings.IsSet.IdleTimeoutMs = TRUE;
    settings.IdleTimeoutMs = 30000;
    settings.IsSet.DisconnectTimeoutMs = TRUE;
    settings.DisconnectTimeoutMs = 10000;
    settings.IsSet.CertValidationFlags = TRUE;
    settings.CertValidationFlags = QUIC_CERTIFICATE_FLAG_DISABLE_CERT_VALIDATION;

    status = MsQuic->ConfigurationOpen(registration_, &alpnBuffer, 1, &settings, sizeof(settings), nullptr, &configuration_);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("ConfigurationOpen failed");
        return false;
    }

    status = MsQuic->ConfigurationLoadCredential(configuration_, nullptr);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("ConfigurationLoadCredential failed");
        return false;
    }

    return true;
}

void QuicClient::start() {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    QUIC_STATUS status;

    status = MsQuic->ConnectionOpen(registration_, ClientConnectionCallback, this, &connection_);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("ConnectionOpen failed");
        return;
    }

    QUIC_ADDR addr = {};
    QuicAddrSetFamily(&addr, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&addr, serverPort_);

    status = MsQuic->ConnectionStart(connection_, configuration_, QUIC_ADDRESS_FAMILY_UNSPEC, serverIp_.c_str(), serverPort_);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("ConnectionStart failed");
        MsQuic->ConnectionClose(connection_);
        connection_ = nullptr;
    } else {
        Logger::getLogger()->info("QUIC connection started to {}:{}", serverIp_, serverPort_);
    }
}

void QuicClient::stop() {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    if (connection_) {
        MsQuic->ConnectionClose(connection_);
        connection_ = nullptr;
    }
    if (configuration_) {
        MsQuic->ConfigurationClose(configuration_);
        configuration_ = nullptr;
    }
}

void QuicClient::sendData(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    if (!connection_) {
        Logger::getLogger()->error("QUIC connection is not established");
        return;
    }

    HQUIC stream = nullptr;
    QUIC_STATUS status;

    status = MsQuic->StreamOpen(connection_, QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL, ClientStreamCallback, this, &stream);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("StreamOpen failed");
        return;
    }

    status = MsQuic->StreamStart(stream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("StreamStart failed");
        MsQuic->StreamClose(stream);
        return;
    }

    QUIC_BUFFER buffer;
    buffer.Length = static_cast<uint32_t>(len);
    buffer.Buffer = const_cast<uint8_t*>(data);

    status = MsQuic->StreamSend(stream, &buffer, 1, QUIC_SEND_FLAG_FIN, nullptr);
    if (QUIC_FAILED(status)) {
        Logger::getLogger()->error("StreamSend failed");
        MsQuic->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        MsQuic->StreamClose(stream);
    }
}

void QuicClient::setDataHandler(std::function<void(const uint8_t* data, size_t len)> handler) {
    dataHandler_ = handler;
}

QUIC_STATUS QUIC_API QuicClient::ClientConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event) {
    QuicClient* client = static_cast<QuicClient*>(Context);
    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        Logger::getLogger()->info("QUIC connected");
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        Logger::getLogger()->warn("QUIC connection shutdown by transport, error code: {}", Event->SHUTDOWN_INITIATED_BY_TRANSPORT.ErrorCode);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        Logger::getLogger()->warn("QUIC connection shutdown by peer");
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        Logger::getLogger()->info("QUIC shutdown complete");
        MsQuic->ConnectionClose(Connection);
        client->connection_ = nullptr;
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
