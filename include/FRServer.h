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
#include "networking/server_interface.h"
#include "constants.h"

using namespace fortress::net;
using FRClient = std::shared_ptr<tcp_connection>;

class FRServer : public server_interface {

private:

    // Emulate ADC readings to send repeatedly
    std::function<void(FRServer *)> m_updateCallback;
    std::unique_ptr<asio::steady_timer> m_pPingTimer;
    std::unique_ptr<asio::steady_timer> m_pUpdateTimer;

    bool m_bIsPinging = false;
    bool m_bIsUpdating = false;

    static constexpr asio::chrono::milliseconds PING_DELAY{ 1000 };
    asio::chrono::milliseconds m_nSamplingPeriodMilliseconds { 1000 };

public:
    explicit FRServer(asio::io_context &io_context, uint16_t port, std::function<void(FRServer *)> updateCallback) :
            server_interface(io_context, port),
            m_updateCallback{ std::move(updateCallback) },
            m_pPingTimer{ std::make_unique<asio::steady_timer>(io_context) },
            m_pUpdateTimer{std::make_unique<asio::steady_timer>(io_context) } {};

protected:
    bool onClientConnect(std::shared_ptr<tcp_connection> client) override;

    void onClientDisconnect(std::shared_ptr<tcp_connection> client) override;

    void onMessage(std::shared_ptr<tcp_connection> client, message<MsgTypes> &msg) override;

public:
    void pingAll();

    void togglePingUpdate();

private:

    void pingAllHandler();

    static void respondToPing(const std::shared_ptr<tcp_connection> &client, const message<MsgTypes> &msg);

    static void onPingReceive(const std::shared_ptr<tcp_connection> &client, message<MsgTypes> &msg);

    void updateHelper();

    void startUpdating(message<MsgTypes> &msg);

    void stopUpdating();
};

#endif //FORTRESS_FR_SERVER_H
