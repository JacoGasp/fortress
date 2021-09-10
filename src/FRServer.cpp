//
// Created by Jacopo Gasparetto on 15/06/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#include "FRServer.h"

// ---- Protected Methods ----

bool FRServer::onClientConnect(std::shared_ptr<tcp_connection> client) {
    std::cout << "[SERVER] New Client Connected\n";
    message<MsgTypes> newMessage;
    newMessage.header.id = ServerAccept;
    newMessage << client->getID();
    sendMessage(client, newMessage);
    std::cout << '[' << client->getID() << "] Client Validated" << std::endl;
    return true;
}

void FRServer::onClientDisconnect(std::shared_ptr<tcp_connection> client) {
    std::cout << '[' << client->getID() << "] Client Disconnected\n";
    if (m_bIsUpdating)
        stopUpdating();
}

void FRServer::onMessage(const FRClient client, message<MsgTypes> &msg) {
    switch (msg.header.id) {
        case ServerPing:
            respondToPing(client, msg);
            break;
        case ClientPing:
            onPingReceive(client, msg);
            break;
        case ClientStartUpdating:
            startUpdating(msg);
            break;
        case ClientStopUpdating:
            stopUpdating();
            break;
        case ClientDisconnect:
            std::cout << '[' << client->getID() << "] Client Disconnects\n";
            break;
        case MessageAll:
            break;
        default:
            std::cout << "???" << std::endl;
    }
}

// ---- Public Methods ----

void FRServer::togglePingUpdate() {
    if (!m_bIsPinging) {
        m_bIsPinging = true;
        pingAllHandler();
    } else {
        m_bIsPinging = false;
    }
}

void FRServer::pingAll() {
    std::cout << "[SERVER]: Ping All\n";
    message<MsgTypes> msg;
    msg.header.id = ClientPing;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    msg << timeNow;
    sendMessageToAllClients(msg);
}

// ---- Private Methods ----

void FRServer::pingAllHandler() {
    if (m_bIsPinging) {
        pingAll();
        m_pPingTimer->expires_from_now(PING_DELAY);
        m_pPingTimer->async_wait([this](asio::error_code ec) {
            pingAllHandler();
        });
    }
}

void FRServer::respondToPing(const std::shared_ptr<tcp_connection> &client, const message<MsgTypes> &msg) {
    // std::cout << '[' << client_interface->getID() << "]: Server Ping\n";
    // Simply bounce message back to client_interface
    client->send(msg);
}

void FRServer::onPingReceive(const std::shared_ptr<tcp_connection> &client, message<MsgTypes> &msg) {
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timeThen;
    msg >> timeThen;
    std::cout << '[' << client->getID() << "] Ping: "
              << std::chrono::duration<double>(timeNow - timeThen).count() * 1000
              << " ms.\n";
}

void FRServer::updateHelper() {
    if (m_bIsUpdating) {
        m_updateCallback(this);
        m_pUpdateTimer->expires_from_now(m_nSamplingPeriodMilliseconds);
        m_pUpdateTimer->async_wait([this](asio::error_code ec) {
            if (!ec) {
                updateHelper();
            } else {
                m_bIsUpdating = false;
                std::cout << "[SEVER] An error occurred during readings update\n";
            }
        });
    }
}

void FRServer::startUpdating(message<MsgTypes> &msg) {
    if (!m_bIsUpdating) {
        uint16_t frequency;
        msg >> frequency;
        auto delay = static_cast<int>(1.0 / frequency * 1'000);
        m_nSamplingPeriodMilliseconds = asio::chrono::milliseconds{ delay };
        m_bIsUpdating = true;
        updateHelper();

        std::cout << "[SERVER]: Start updating width sampling period of " << m_nSamplingPeriodMilliseconds.count()
                  << " ms.\n";
    }
}

void FRServer::stopUpdating() {
    m_bIsUpdating = false;
    std::cout << "[SERVER]: Stop updating\n";
}
