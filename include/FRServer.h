//
// Created by Jacopo Gasparetto on 15/06/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#ifndef FORTRESS_FR_SERVER_H
#define FORTRESS_FR_SERVER_H

#include <iostream>
#include <thread>
#include <utility>
#include "Networking/server.h"
#include "Constants.h"

using namespace fortress::net;
using FRClient = std::shared_ptr<Connection<MsgTypes>>;

class FRServer : public ServerInterface<MsgTypes> {
private:
    std::thread m_thUpdate;
    std::atomic_bool m_bIsUpdating{};
    int m_nSamplingPeriodsMicroseconds{ 10'000};
    std::function<void(FRServer *)> m_updateCallback;

public:
    explicit FRServer(uint16_t port, std::function<void(FRServer *)> updateCallback) :
            ServerInterface<MsgTypes>(port), m_updateCallback{std::move( updateCallback )} {};

protected:
    bool onClientConnect(std::shared_ptr<Connection<MsgTypes>> client) override;

    void onClientDisconnect(std::shared_ptr<Connection<MsgTypes>> client) override;

    void onClientValidated(std::shared_ptr<Connection<MsgTypes>> client) override;

    void onMessage(std::shared_ptr<Connection<MsgTypes>> client, message<MsgTypes> &msg) override;

public:
    void pingAll();

private:
    static void respondToPing(const std::shared_ptr<Connection<MsgTypes>> &client, const message<MsgTypes> &msg);

    static void onPingReceive(const std::shared_ptr<Connection<MsgTypes>> &client, message<MsgTypes> &msg);

    void updateHelper();

    void startUpdating(message<MsgTypes> &msg);

    void stopUpdating();
};

#endif //FORTRESS_FR_SERVER_H
