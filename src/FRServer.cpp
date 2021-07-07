//
// Created by Jacopo Gasparetto on 15/06/21.
//

#include "FRServer.h"

bool FRServer::onClientConnect(std::shared_ptr<Connection<MsgTypes>> client) {
    std::cout << "[SERVER] New Client Connected\n";
    return true;
}

void FRServer::onClientDisconnect(std::shared_ptr<Connection<MsgTypes>> client) {
    std::cout << '[' << client->getID() << "] Client Disconnected\n";
}

void FRServer::onClientValidated(FRClient client) {
    message<MsgTypes> newMessage;
    newMessage.header.id = ServerAccept;
    newMessage << client->getID();
    sendMessage(client, newMessage);
    std::cout << '[' << client->getID() << "] Client Validated" << std::endl;
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

void FRServer::pingAll() {
    std::cout << "[SERVER]: Ping All\n";
    message<MsgTypes> msg;
    msg.header.id = ClientPing;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    msg << timeNow;
    sendMessageToAllClients(msg);
}

void FRServer::respondToPing(const std::shared_ptr<Connection<MsgTypes>> &client, const message<MsgTypes> &msg) {
    std::cout << '[' << client->getID() << "]: Server Ping\n";
    //Simply bounce message back to client
    client->send(msg);
}

void FRServer::onPingReceive(const std::shared_ptr<Connection<MsgTypes>> &client, message<MsgTypes> &msg) {
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timeThen;
    msg >> timeThen;
    std::cout << '[' << client->getID() << "] Ping: "
              << std::chrono::duration<double>(timeNow - timeThen).count() * 1000
              << " ms.\n";
}

void FRServer::updateHelper() {
    while (m_bIsUpdating) {
        m_updateCallback(this);
        std::this_thread::sleep_for(std::chrono::microseconds(m_nSamplingPeriodsMicroseconds));
    }
}

void FRServer::startUpdating(message<MsgTypes> &msg) {
    if (!m_bIsUpdating) {
        double frequency;
        msg >> frequency;
        m_nSamplingPeriodsMicroseconds = static_cast<int>(1 / frequency * 1'000'000);
        m_bIsUpdating = true;
        m_thUpdate = std::thread{ &FRServer::updateHelper, this };
        std::cout << "[SERVER]: Start updating width sampling period of " << m_nSamplingPeriodsMicroseconds / 1000
                  << " ms.\n";
    }
}

void FRServer::stopUpdating() {
    m_bIsUpdating = false;
    m_thUpdate.join();
    std::cout << "[SERVER]: Stop updating\n";
}
