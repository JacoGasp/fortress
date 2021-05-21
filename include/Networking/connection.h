//
// Created by Jacopo Gasparetto on 19/05/21.
//

#ifndef FORTRESS_CONNECTION_H
#define FORTRESS_CONNECTION_H

#include "threadsafe_queue.h"
#include "message.h"

namespace fortress::net {

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
        // Each connection has a unique socket to a remote
        asio::ip::tcp::socket m_socket;

        // This context is shared with the whole asio instance
        asio::io_context &m_asioContext;

        // This queue holds all messages to be sent to the remote side of this connection
        threadSafeQueue<message<T>> m_qMessagesOut;

        // This queue holds all messages that have been received from the remote side of this connection.
        // It is a reference as the "owner" of this connection is expected to provide a queue.
        threadSafeQueue<owned_message<T>> &m_qMessagesIn;

        // The owner decides how some of the connection behaves
        owner m_ownerType = owner::server;

        message<T> m_msgTemporaryIn;

        uint32_t m_id = 0;

    public:

        Connection(owner parent, asio::io_context &asioContext,
                   asio::ip::tcp::socket socket, threadSafeQueue<owned_message<T>> &qIn)
                : m_asioContext{ asioContext },
                  m_socket{ std::move(socket) },
                  m_qMessagesIn{ qIn } {

            m_ownerType = parent;

        }

        virtual ~Connection() = default;

        [[nodiscard]] uint32_t getID() const {
            return m_id;
        }

    public:

        void connectToClient(uint32_t uid = 0) {

            if (m_ownerType == owner::server)
                if (m_socket.is_open()) {
                    m_id = uid;
                    readHeader(); // Start polling client
                }
        }

        void connectToServer(const asio::ip::tcp::resolver::results_type &endpoints) {
            if (m_ownerType == owner::client) {
                asio::async_connect(m_socket, endpoints,
                                    [this](std::error_code ec, const asio::ip::tcp::endpoint &endpoint) {
                                        if (!ec) {
                                            readHeader();
                                        }
                                    });
            }

        }

        void disconnect() {
            if (isConnected())
                asio::post(m_asioContext, [this]() { m_socket.close(); });
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

        void reset() {}


    private:
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
        // Async - Prime context ready to read a message header
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

                                 } else {
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
    };
}

#endif //FORTRESS_CONNECTION_H
