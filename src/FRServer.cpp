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
        case ClientMessage:
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

void FRServer::respondToPing(const std::shared_ptr<Connection<MsgTypes>>& client, const message<MsgTypes> &msg) {
    std::cout << '[' << client->getID() << "]: Server Ping\n";
    //Simply bounce message back to client
    client->send(msg);
}

void FRServer::onPingReceive(const std::shared_ptr<Connection<MsgTypes>>& client, message<MsgTypes> &msg) {
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timeThen;
    msg >> timeThen;
    std::cout <<  '[' << client->getID() << "] Ping: "
              << std::chrono::duration<double>(timeNow - timeThen).count() * 1000
              << " ms.\n";
}

void FRServer::onSetSampleFrequency(const message<MsgTypes> &msg) {

}

//void FRServer::start() {
//    ServerInterface::start();
//}
//
//void FRServer::stop() {
//    ServerInterface::stop();
//}
