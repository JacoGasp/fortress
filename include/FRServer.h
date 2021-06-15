//
// Created by Jacopo Gasparetto on 15/06/21.
//

#ifndef FORTRESS_FR_SERVER_H
#define FORTRESS_FR_SERVER_H

#include <iostream>
#include "Networking/server.h"
#include "Constants.h"

using namespace fortress::net;
using FRClient = std::shared_ptr<Connection<MsgTypes>>;

class FRServer : public ServerInterface<MsgTypes> {
public:
    explicit FRServer(uint16_t port) : ServerInterface<MsgTypes>(port){};

protected:
    bool onClientConnect(std::shared_ptr<Connection<MsgTypes>> client) override;

    void onClientDisconnect(std::shared_ptr<Connection<MsgTypes>> client) override;

    void onClientValidated(std::shared_ptr<Connection<MsgTypes>> client) override;

    void onMessage(std::shared_ptr<Connection<MsgTypes>> client, message<MsgTypes> &msg) override;

public:
    void pingAll();

private:
    static void respondToPing(const std::shared_ptr<Connection<MsgTypes>>& client, const message<MsgTypes> &msg);

    static void onPingReceive(const std::shared_ptr<Connection<MsgTypes>>& client, message<MsgTypes> &msg);

    void onSetSampleFrequency(const message<MsgTypes> &msg);

//    void start();
//
//    void stop();
};

#endif //FORTRESS_FR_SERVER_H
