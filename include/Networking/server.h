//
// Created by Jacopo Gasparetto on 19/05/21.
//

#ifndef FORTRESS_SERVER_H
#define FORTRESS_SERVER_H

#include <iostream>
#include <cstdint>
#include <memory>
#include <deque>

#include "message.h"
#include "connection.h"
#include "threadsafe_queue.h"

namespace fortress::net {

    template<typename T>
    class ServerInterface {

    protected:
        // Keep order since is the initialization order
        threadSafeQueue<owned_message<T>> m_qMessagesIn;             // Thread safe queue for incoming message packets
        std::deque<std::shared_ptr<Connection<T>>> m_connections;    // Container of active connections
        asio::io_context m_asioContext;                              // Asio Context
        std::thread m_threadContext;                                 // Thread Context
        asio::ip::tcp::acceptor m_asioAcceptor;                      // Used to accept new socket connections

        uint32_t nIDCounter = 10000;                                 // Client unique identifier
        uint16_t m_port;                                             // Only for verbose

    protected:
        virtual bool onClientConnect(std::shared_ptr<Connection<T>> client) { return false; }

        virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client) {}

        virtual void onMessage(std::shared_ptr<Connection<T>> client, message<T> &msg) {}

    public:
        // Constructors

        explicit ServerInterface(uint16_t port)
                : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
                  m_port(port) {}

        virtual ~ServerInterface() {
            stop();
        }

        // Helpers
        // Start the server
        bool start() {
            try {

                waitForClientConnection();  // We guarantee that there is a task and keep the context alive
                m_threadContext = std::thread([this]() { m_asioContext.run(); });

            } catch (std::exception &e) {
                std::cerr << "[SERVER] Exception: " << e.what() << '\n';
                return false;
            }

            std::cout << "[SERVER] Started at port " << m_port << "!\n";
            return true;
        }

        void stop() {
            m_asioContext.stop();

            // Clean the context thread
            if (m_threadContext.joinable())
                m_threadContext.join();

        }

        void waitForClientConnection() {
            m_asioAcceptor.template async_accept(
                    [this](std::error_code ec, asio::ip::tcp::socket socket) {

                        if (!ec) {
                            std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << '\n';

                            auto newConnection = std::make_shared<Connection<T>>(
                                    Connection<T>::owner::server,
                                    m_asioContext,
                                    std::move(socket),
                                    m_qMessagesIn
                            );

                            if (onClientConnect(newConnection)) {
                                // Connection allowed, add it to active connections
                                m_connections.push_back(std::move(newConnection));
                                m_connections.back()->connectToClient(nIDCounter++);

                                std::cout << "[" << m_connections.back()->getID() << "] Connection Approved\n";

                            } else {
                                std::cout << "[-----] Connection Denied\n";
                            }

                        } else {
                            // Error has occurred during acceptance
                            std::cout << "[SERVER] New Connection Error: " << ec.message() << '\n';
                        }

                        // Prime th asio context with more work - again simply wait for another connection...
                        waitForClientConnection();
                    });
        }

        void sendMessage(std::shared_ptr<Connection<T>> client, const message<T> &msg) {
            if (client && client->isConnected())
                client->send(msg);
            else {
                onClientDiconnect(client);

                client.reset();

                // Physically remove the client from connections
                m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client),
                                    m_connections.end());
            }
        }

        void sendMessageToAllClients(const message<T> &msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) {
            bool bInvalidClientExists = false;

            // For each client check if it is connected and, if it must be not ignored, send the message to it.
            // If the client is not
            for (auto &client : m_connections)
                if (client && client->isConnected()) {
                    if (client != pIgnoreClient)
                        client->send(msg);
                } else {
                    onClientConnect(client);
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

        /*
         *
         */
        void update(size_t nMaxMessages = -1, bool bWait = false) {  // set to maximum size_t by default
            if (bWait)
                m_qMessagesIn.wait();

            size_t nMessageCount = 0;

            while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
                // Grab the front message
                auto msg = m_qMessagesIn.pop_front();

                // Pass to message handler
                onMessage(msg.remote, msg.message);
                nMessageCount++;
            }
        }
    };
}

#endif //FORTRESS_SERVER_H
