//
// Created by Jacopo Gasparetto on 15/06/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#ifndef FORTRESS_ESP32_SERVER_H
#define FORTRESS_ES32_SERVER_H

#include <freertos/FreeRTOS.h>
#include <AsyncTCP.h>
#include <deque>
#include <cassert>
#include "../../include/networking/message.h"
#include "../../include/constants.h"
// #include "ThreadSafeQueue.h"

using Message = fortress::net::message<fortress::net::MsgTypes>;
using Header = fortress::net::message_header<fortress::net::MsgTypes>;

class TCPServer {

private:
    AsyncServer m_server;
    size_t m_expectedBodyLength = 0;
    bool m_isWriting = false;
    
    Message m_tempInMessage;
    std::deque<Message> m_qMessagesOut;
    // fortress::esp32::taskSafeQueue<Message> m_qMessagesOut;
    std::function<void(Message&, AsyncClient*)> m_onMessageCallback;

public:
    TCPServer(uint16_t port);

    void begin();

    void end();

    void sendMessage(const Message &msg, AsyncClient * client);

    void setOnMessageCallback(std::function<void(Message&, AsyncClient*)> callback);

protected:
    void onConnect(void *arg, AsyncClient *client);

    void onData(void *arg, AsyncClient *client, void *data, size_t len);

    void onMessage(Message &msg, AsyncClient *client);

private:
    void writeHeader(AsyncClient *client);

    void writeBody(AsyncClient *client);

    void readHeader(uint8_t *data);

    void readBody(uint8_t *data);

    void printData(uint8_t *data, size_t len);
};

#endif //FORTRESS_ESP_SERVER_H
