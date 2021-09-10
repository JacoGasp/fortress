//
// Created by Jacopo Gasparetto on 10/08/21.
//

#ifndef FORTRESS_SERVER_INTERFACE_H
#define FORTRESS_SERVER_INTERFACE_H

#include "networking/tcp_connection.h"

namespace fortress::net {
    class server_interface {

    private:
        asio::io_context &m_context;
        asio::ip::tcp::acceptor m_acceptor;
        std::deque<std::shared_ptr<tcp_connection>> m_connections;

        uint32_t m_id_counter{ 0 };
        uint16_t m_port;

    protected:
        virtual bool onClientConnect(std::shared_ptr<tcp_connection> client) = 0;

        virtual void onClientDisconnect(std::shared_ptr<tcp_connection> client) = 0;

        virtual void onMessage(std::shared_ptr<tcp_connection> client, message<MsgTypes> &msg) = 0;


    public:
        explicit server_interface(asio::io_context &context, uint16_t port)
                : m_context{ context },
                  m_acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
                  m_port(port) {}

        virtual ~server_interface() {
            stop();
        }

        void sendMessage(std::shared_ptr<tcp_connection> client, const message<MsgTypes> &msg) {
            if (client && client->isConnected()) {
                client->send(msg);
            } else {
                onClientDisconnect(client);
                client.reset();
                m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
            }
        }

        void sendMessageToAllClients(const message<MsgTypes> &msg, std::shared_ptr<tcp_connection> pIgnoreClient = nullptr) {
            bool bInvalidClientExists = false;

            // For each client_interface check if it is connected and, if it must be not ignored, send the message to it.
            // If the client_interface is not
            for (auto &client : m_connections)
                if (client && client->isConnected()) {
                    if (client != pIgnoreClient)
                        client->send(msg);
                } else {
                    onClientDisconnect(client);
                    client.reset();
                    bInvalidClientExists = true;
                }

            if (bInvalidClientExists)
                // Search for connections equal to nullptr e remove it from the connections queue
                m_connections.erase(
                        std::remove(m_connections.begin(), m_connections.end(), nullptr),
                        m_connections.end()
                );
        }

        bool start() {
            try {
                waitForClientToConnect();
            } catch (std::exception &e) {
                std::cerr << "[SERVER] Exception: " << e.what() << '\n';
                return false;
            }

            std::cout << "[SERVER] Started at port " << m_port << "!\n";
            return true;
        }

        void stop() {
            std::cout << "Stopping server...\n";
            for (auto &conn : m_connections)
                conn->disconnect();
        }

    private:
        void waitForClientToConnect() {
            m_acceptor.template async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    std::cout << "[SERVER] New Connection from " << socket.remote_endpoint() << '\n';

                    auto newConnection = std::make_shared<tcp_connection>(
                            m_context,
                            std::move(socket),
                            tcp_connection::owner::server,
                            [this](owned_message<MsgTypes> &msg) { onMessage(msg.remote, msg.message); }
                    );

                    if (onClientConnect(newConnection)) {
                        m_connections.push_back(std::move(newConnection));
                        m_connections.back()->connectToClient(m_id_counter++);
                        std::cout << "[" << m_connections.back()->getID() << "] Connection Approved\n";
                    } else {
                        std::cout << "[-----] Connection Denied\n";
                    }
                } else {
                    std::cout << "[SERVER] Error while new connection attempt: " << ec.message() << '\n';
                }

                // Wait for a new connection
                waitForClientToConnect();
            });
        }

    };

}
#endif //FORTRESS_SERVER_INTERFACE_H
