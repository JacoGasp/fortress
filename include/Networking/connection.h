//
// Created by Jacopo Gasparetto on 19/05/21.
//

#ifndef FORTRESS_CONNECTION_H
#define FORTRESS_CONNECTION_H

#include "threadsafe_queue.h"
#include "message.h"

namespace fortress::net {

    // Forward declare
    template<typename T>
    class ServerInterface;

    template<typename T>
    /*
      Enable the class Connection<T>, currently managed by a std::shared_pointer name pt, to safely generate
      additional std::shared_ptr instances pt1, pt2, ... that all share ownership of t with pt.

     class Foo : public std::enable_shared_from_this<Foo> {
     public:
        std::shared_ptr<Foo> getPtr() { return shared_from_this();}
     }

      std::shared_ptr<Foo> foo1 = std::make_shared<Foo>();
      std::shared_ptr<Foo> foo2 = foo1->getPtr();
    */
    class Connection : public std::enable_shared_from_this<Connection<T>> {

    public:
        enum class owner {
            server,
            client
        };

    protected:
        // This context is shared with the whole asio instance
        asio::io_context &m_asioContext;

        // Each connection has a unique socket to a remote
        asio::ip::tcp::socket m_socket;

        // This queue holds all messages to be sent to the remote side of this connection
        threadSafeQueue<message<T>> m_qMessagesOut;

        // This queue holds all messages that have been received from the remote side of this connection.
        // It is a reference as the "owner" of this connection is expected to provide a queue.
        threadSafeQueue<owned_message<T>> &m_qMessagesIn;

        // The owner decides how some of the connection behaves
        owner m_ownerType = owner::server;

        message<T> m_msgTemporaryIn;

        // Connection id
        uint32_t m_id = 0;

        // Handshake Validation
        uint64_t m_nHandshakeOut = 0;
        uint64_t m_nHandshakeIn = 0;
        uint16_t m_nHandshakeCheck = 0;

    public:

        Connection(owner parent, asio::io_context &asioContext,
                   asio::ip::tcp::socket socket, threadSafeQueue<owned_message<T>> &qIn)
                : m_asioContext{ asioContext },
                  m_socket{ std::move(socket) },
                  m_qMessagesIn{ qIn } {

            m_ownerType = parent;

            // Construct validation check data
            if (m_ownerType == owner::server) {
                // Server -> Client
                // Construct random data to send for validation
                m_nHandshakeOut = u_int64_t(std::chrono::system_clock::now().time_since_epoch().count());
                // Precalculate the expected response
                m_nHandshakeCheck = scramble(m_nHandshakeOut);
            } else {
                // Client -> Server
                m_nHandshakeIn = 0;
                m_nHandshakeCheck = 0;
            }
        }

        virtual ~Connection() = default;

        [[nodiscard]] uint32_t getID() const {
            return m_id;
        }

    public:
        // Implemented in server.h only. Makes no sense in client.h
        void onClientValidate([[maybe_unused]] fortress::net::ServerInterface<T> *server);

        void connectToClient(fortress::net::ServerInterface<T> *server, uint32_t uid = 0) {

            if (m_ownerType == owner::server)
                if (m_socket.is_open()) {
                    m_id = uid;

                    // A client has attempted to connect to the server. Write out handshake data to be validated
                    writeValidation();
                    // Listen asynchronously for validation data sent back
                    readValidation(server);
                }
        }

        void connectToServer(const asio::ip::tcp::resolver::results_type &endpoints) {
            if (m_ownerType == owner::client) {
                asio::async_connect(m_socket, endpoints,
                                    [this](std::error_code ec, const asio::ip::tcp::endpoint &endpoint) {
                                        if (!ec) {
                                            readValidation();
                                        } else {
                                            std::cout << "Failed to connect to server: " << ec.message() << std::endl;
                                            m_socket.close();
                                        }
                                    });
            }

        }

        void disconnect() {
            if (isConnected())
                asio::post(m_asioContext, [this]() {
                    m_socket.shutdown(asio::socket_base::shutdown_both);
                    m_socket.close();
                    m_socket.release();
                });
        }

        [[nodiscard]] bool isConnected() const {
            return m_socket.is_open();
        }

    public:
        void send(const message<T> &msg) {
            // Post the message to the asio context
            asio::post(m_asioContext, [this, msg]() {

                bool isWritingMessage = !m_qMessagesOut.empty();
                m_qMessagesOut.push_back(msg);

                // If the queue was not empty before inserting a new message, asio is still busy to finish sending
                // previous messages
                if (!isWritingMessage)
                    writeHeader();
            });
        }


    private:

        void writeValidation() {
            asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
                              [this](std::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      // Validation data sent, clients should sit and wait for a response
                                      if (m_ownerType == owner::client)
                                          readHeader();
                                  } else {
                                      m_socket.close();
                                  }
                              });
        }

        void writeHeader() {
            asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
                              [this](std::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      if (m_qMessagesOut.front().body.size() > 0)
                                          writeBody();
                                      else {
                                          // If the message is empty pop it out from the queue
                                          m_qMessagesOut.pop_front();

                                          // If the queue is not empty, there are other messages to send
                                          // and issue the task sending the next header
                                          if (!m_qMessagesOut.empty())
                                              writeHeader();
                                      }
                                  } else {
                                      std::cout << '[' << m_id << "] Write header failed: " << ec.message() << '\n';
                                      m_socket.close();
                                  }
                              });
        }

        void writeBody() {
            asio::async_write(
                    m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                    [this](std::error_code ec, std::size_t length) {
                        if (!ec) {
                            // If the message body has been successfully written into the socket,
                            // pop it out from the queue
                            m_qMessagesOut.pop_front();

                            // If the queue is still not empty, write the next message header
                            if (!m_qMessagesOut.empty())
                                writeHeader();

                        } else {
                            std::cout << '[' << m_id << "] Write body failed: " << ec.message() << '\n';
                            m_socket.close();
                        }
                    });
        }

        void readValidation(fortress::net::ServerInterface<T> *server = nullptr) {
            asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
                             [this, server](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     if (m_ownerType == owner::server) {
                                         if (m_nHandshakeIn == m_nHandshakeCheck) {
                                             // Client provided valid handshake
                                             onClientValidate(server);

                                             // Sit waiting to receive data now
                                             readHeader();
                                         } else {
                                             // Handshake failed
                                             std::cout << "[SERVER] Client Disconnected (Failed Validation)"
                                                       << std::endl;
                                             m_socket.close();
                                         }
                                     } else {
                                         // Connection is client, resolve handshake
                                         m_nHandshakeOut = scramble(m_nHandshakeIn);
                                         writeValidation();
                                     }
                                 } else {
                                     std::cout << "[SERVER] Client Disconnected (readValidation): "
                                     << ec.message() << std::endl;
                                     m_socket.close();
                                 }
                             });
        }

        void readHeader() {
            // Read the context and write the m_msgTemporary's header
            asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
                             [this](std::error_code ec, std::size_t length) {

                                 if (!ec) {
                                     // If there is data, resize the body and write data to it
                                     if (m_msgTemporaryIn.header.size > 0) {
                                         m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                         readBody();
                                     } else {
                                         addToIncomingMessageQueue();
                                     }

                                 } else if (ec.value() == asio::error::misc_errors::eof) {
                                     std::cout << "Peer disconnected" << std::endl;
                                     m_socket.close();
                                 }

                                 else {
                                     std::cout << '[' << m_id << "] Read header failed: " << ec.message() << '\n';
                                     m_socket.close();
                                 }
                             });
        }

        void readBody() {
            asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                             [this](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     addToIncomingMessageQueue();
                                 } else {
                                     std::cout << '[' << m_id << "] Read body failed: " << ec.message() << '\n';
                                     m_socket.close();
                                 }
                             });
        }

        /* If the owner is the server, transform the message in a owned message and push it to the incoming queue
         * The owned_message class whats a shared_ptr<Connection<T>> remote and a message<T>
         * and we pass a shared pointer of this (connection) class
         */
        void addToIncomingMessageQueue() {
            if (m_ownerType == owner::server)
                m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });      // Initializer list

            else
                // One client have one and only one connection to a single remote, and we don't need to specify it in
                // each message. Instead of a shared_ptr we use a unique_ptr
                m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

            // After the message has been added to the queue, read a new message
            readHeader();
        }

        // Encrypt data
        uint16_t scramble(uint16_t nInput) {
            uint64_t out = nInput ^0xDEADBEEFC0DECADE;
            out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0xF0F0F0F0F0F0F0 << 4);
            return out ^ 0xC0DEFACE12345678;
        }
    };
}

#endif //FORTRESS_CONNECTION_H
