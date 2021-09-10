//
// Created by Jacopo Gasparetto on 11/08/21.
//

#ifndef FORTRESS_CLIENT_INTERFACE_H
#define FORTRESS_CLIENT_INTERFACE_H

#include "networking/tcp_connection.h"
#include <functional>

namespace fortress::net {

    class client_interface {
    private:
        asio::io_context &m_context;
        std::unique_ptr<tcp_connection> m_connection;

    public:
        explicit client_interface(asio::io_context &context) : m_context{ context } {}

    protected:
        virtual void onMessage(message<MsgTypes> &msg) = 0;

    public:
        bool connect(const std::string &host, const uint16_t port) {
            std::cout << "Connecting to server: " << host << ':' << port << '\n';

            try {
                asio::ip::tcp::resolver resolver{ m_context };
                auto endpoints = resolver.resolve(host, std::to_string(port));

                m_connection = std::make_unique<tcp_connection>(
                        m_context,
                        asio::ip::tcp::socket{ m_context },
                        tcp_connection::owner::client,
                        [this](owned_message<MsgTypes> &msg) { onMessage(msg.message); }
                );

                m_connection->connectToServer(endpoints);
            } catch (std::exception &e) {
                std::cerr << "Client Exception: " << e.what() << "\n";
                return false;
            }

            return true;
        }

        bool isConnected() {

            if (m_connection) {
                return m_connection->isConnected();
            }
            return false;
        }

        void disconnect() {
            asio::dispatch(m_context, [this]() { m_connection->disconnect(); });
        }

        void sendMessage(message<MsgTypes> &msg) const {
            m_connection->send(msg);
        }
    };
}

#endif //FORTRESS_CLIENT_INTERFACE_H
