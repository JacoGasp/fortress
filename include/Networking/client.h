//
// Created by Jacopo Gasparetto on 19/05/21.
//

#ifndef FORTRESS_CLIENT_H
#define FORTRESS_CLIENT_H

#include "commons.h"
#include "connection.h"
#include "threadsafe_queue.h"
#include "message.h"

namespace fortress::net {

    template<typename T>
    class ClientInterface {

    private:
        threadSafeQueue<owned_message<T>> m_qMessageIn;

    protected:
        // asio context handles the data transfer
        asio::io_context m_context;

        std::thread m_threadContext;

        std::unique_ptr<Connection<T>> m_connection;

    public:
        ClientInterface() = default;

        virtual ~ClientInterface() {
            disconnect();
        }

        bool connect(const std::string &host, const uint16_t port) {
            std::cout << "Connecting to server: " << host << ':' << port << '\n';
            try {
                // Resolve hostname/ip-address into physical address
                asio::ip::tcp::resolver resolver(m_context);
                auto endpoints = resolver.resolve(host, std::to_string(port));

                // Create connection
                m_connection = std::make_unique<Connection<T>>(
                        Connection<T>::owner::client,
                        m_context,
                        asio::ip::tcp::socket(m_context),
                        m_qMessageIn
                );

                // Tell the connection object to connect to server
                m_connection->connectToServer(endpoints);

                // Start the thread context
                m_threadContext = std::thread([this]() { m_context.run(); });

            } catch (std::exception &e) {
                std::cerr << "Client Exception: " << e.what() << "\n";
            }

            return false;
        }

        void disconnect() {

            // Disconnect gracefully
            if (isConnected())
                m_connection->disconnect();

            // Stop the context
            m_context.stop();

            // Stop context thread
            if (m_threadContext.joinable())
                m_threadContext.join();

            // Destroy the connection
            m_connection.release();
        }

        bool isConnected() {

            if (m_connection) {
                return m_connection->isConnected();
            }
            return false;
        }

        void send(const message<T> &msg) {
            if (isConnected())
                m_connection->send(msg);
        }

        // Retrieve messages queue from server
        threadSafeQueue<owned_message<T>> &incoming() {
            return m_qMessageIn;
        }
    };
}

#endif //FORTRESS_CLIENT_H
