//
// Created by Jacopo Gasparetto on 11/08/21.
//

#ifndef FORTRESS_CLIENT_V2_H
#define FORTRESS_CLIENT_V2_H

#include "Networking/connections_v2.h"
#include <functional>

namespace fortress::net {

    class client_v2 {
    private:
        asio::io_context &m_context;
        std::unique_ptr<ConnectionV2> m_connection;

    public:
        explicit client_v2(asio::io_context &context) : m_context{ context } {}

    public:
        bool connect(const std::string &host, const uint16_t port) {

            asio::ip::tcp::resolver resolver{ m_context };
            auto endpoints = resolver.resolve(host, std::to_string(port));


//            m_connection = std::make_unique<ConnectionV2>(
//                    m_context,
//                    asio::ip::tcp::socket{ m_context },
//                    [this](message<MsgTypes> &msg) { onMessage(msg); }
//                    );
//
//            return true;
            m_connection = std::make_unique<ConnectionV2>(
                    m_context,
                    asio::ip::tcp::socket { m_context },
                    [this](message<MsgTypes> &msg) { onMessage(msg); }
                    );

            m_connection->connectToServer(endpoints);

        }

        void disconnect() {
            asio::post(m_context, [this]() { m_connection->disconnect(); });
        }

        void sendMessage(message<MsgTypes> &msg) {
            m_connection->send(msg);
        }

    protected:
        void onMessage(message<MsgTypes> &msg) {
            std::cout << msg << '\n';
        };
    };
}

#endif //FORTRESS_CLIENT_V2_H
