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
        threadSafeQueue<owned_message<T>> m_qMessagesIn;

    protected:
        // asio context handles the data transfer
        asio::io_context m_context;

        std::thread m_threadContext;

        std::unique_ptr<Connection<T>> m_connection;

        std::thread m_updateThread;

        std::atomic_bool m_bIsListening = false;

    protected:
        virtual void onMessage(message<T> &msg) {};

    public:
        ClientInterface() = default;

        virtual ~ClientInterface() {
            disconnect();
        }

        bool connect(const std::string &host, const uint16_t port) {
            std::cout << "Connecting to server: " << host << ':' << port << '\n';
            try {

                // It should never be the case
                if (m_context.stopped())
                    m_context.restart();

                // Resolve hostname/ip-address into physical address
                asio::ip::tcp::resolver resolver(m_context);
                auto endpoints = resolver.resolve(host, std::to_string(port));

                // Create connection
                m_connection = std::make_unique<Connection<T>>(
                        Connection<T>::owner::client,
                        m_context,
                        asio::ip::tcp::socket(m_context),
                        m_qMessagesIn
                );

                // Tell the connection object to connect to server
                m_connection->connectToServer(endpoints);

                // Start the thread context
                m_threadContext = std::thread([this]() { m_context.run(); });

            } catch (std::exception &e) {
                std::cerr << "Client Exception: " << e.what() << "\n";
                return false;
            }

            return true;
        }

        void disconnect() {

            if (m_bIsListening)
                stopListening();

            // Disconnect gracefully
            if (isConnected())
                m_connection->disconnect();

            // Stop the context
            m_context.restart();

            // Stop the context thread and wait until it finishes
            if (m_threadContext.joinable())
                m_threadContext.join();

            // Close the socket only after the all async work is completed
            m_connection->closeSocket();

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

        void startListening() {
            if (!m_bIsListening) {
                m_updateThread = std::thread{ &ClientInterface<T>::updateHandler, this};
                m_bIsListening = true;
                std::cout << "Start listening remote host...\n";
            } else
                std::cout << "Updating already started";
        }

        void stopListening() {
            if (m_bIsListening) {
                m_bIsListening = false;
                m_qMessagesIn.stopWaiting();

                if (m_updateThread.joinable())
                    m_updateThread.join();

            }
        }

        // Retrieve messages queue from server
        threadSafeQueue<owned_message<T>> &incoming() {
            return m_qMessagesIn;
        }

        // Single shot update
        void update(size_t nMaxMessages = -1, bool bWait = false) {  // set to maximum size_t by default
            if (bWait)
                m_qMessagesIn.wait();

            size_t nMessageCount = 0;

            while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
                // Grab the front message
                auto msg = m_qMessagesIn.pop_front();

                // Pass to message handler
                onMessage(msg.message);
                nMessageCount++;
            }
        }

    protected:
        void updateHandler() {
            while (m_bIsListening) {
                update(-1, true);
            }
        }
    };

    template<typename T>
    // Since we own an object of type Connection<T>, we need to provide an dummy implementation of the onClientValidated
    // function (used only in server.h) to overcome linking errors.
    void Connection<T>::onClientValidate([[maybe_unused]] fortress::net::ServerInterface<T> *server) {}

}

#endif //FORTRESS_CLIENT_H
