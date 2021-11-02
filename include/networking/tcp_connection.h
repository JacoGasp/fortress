//
// Created by Jacopo Gasparetto on 10/08/21.
//

#ifndef FORTRESS_TCP_CONNECTION_H
#define FORTRESS_TCP_CONNECTION_H

#include <utility>

#include "commons.h"
#include "message.h"
#include "threadsafe_queue.h"
#include "constants.h"

namespace fortress::net {

    class server_interface;

    class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
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
        owner m_owner;
        std::function<void(owned_message<MsgTypes> &)> m_onMessageCallback;

        message<MsgTypes> m_tempInMessage;
        ts_queue<message<MsgTypes>> m_qMessagesOut;
        uint32_t m_id{ 0 };


    public:
        tcp_connection(asio::io_context &asioContext, asio::ip::tcp::socket socket, tcp_connection::owner owner,
                       std::function<void(owned_message<MsgTypes> &)> callback) :
                m_asioContext{ asioContext },
                m_socket{ std::move(socket) },
                m_owner{ owner },
                m_onMessageCallback(std::move(callback)) {}

        [[nodiscard]] bool isConnected() const {
            return m_socket.is_open();
        }

        void connectToServer(const asio::ip::tcp::resolver::results_type &endpoints) {
            asio::async_connect(m_socket, endpoints,
                                [this](std::error_code ec, const asio::ip::tcp::endpoint &endpoint) {
                                    if (!ec) {
                                        std::cout << "Connected to: " << endpoint.address().to_string() << '\n';
                                        readHeader();
                                    } else {
                                        std::cout << "Failed to connected with error: " << ec.message() << std::endl;
                                        m_socket.close();
                                    }
                                });
        }

        void connectToClient(uint32_t nID) {
            if (m_socket.is_open()) {
                m_id = nID;
                readHeader();
            }
        }

        void disconnect() {
            if (isConnected()) {
                asio::post(m_asioContext, [this]() {
                    m_socket.shutdown(asio::socket_base::shutdown_both);
                });
            }
        }

        void closeSocket() {
            m_socket.close();
        }

    public:
        void send(const message<MsgTypes> &msg) {
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

        uint32_t getID() const {
            return m_id;
        }

    private:
        void writeHeader() {
            asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<MsgTypes>)),
                              [this](std::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      if (!m_qMessagesOut.front().body.empty())
                                          writeBody();
                                      else {
                                          // If the message is empty pop it out from the queue
                                          m_qMessagesOut.pop_front();

                                          // If the queue is not empty, there are other messages to send
                                          // and issue the task sending the next header
                                          if (!m_qMessagesOut.empty())
                                              writeHeader();
                                      }
                                      return;
                                  }

                                  switch (ec.value()) {
                                      case asio::error::broken_pipe:
                                          std::cout << "Pipe closed\n";
                                          break;
                                      default:
                                          std::cout << '[' << m_id << "] Write header failed: " << ec.message() << '\n';
                                  }
                                  m_socket.close();
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


        void readHeader() {
            asio::async_read(m_socket, asio::buffer(&m_tempInMessage.header, sizeof(message_header<MsgTypes>)),
                             [this](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     if (m_tempInMessage.header.size > 0) {
                                         m_tempInMessage.body.resize(m_tempInMessage.header.size);
                                         readBody();
                                     } else {
                                         onMessage();
                                         readHeader();
                                         m_tempInMessage.body.clear();
                                     }
                                 } else {
                                     std::cout << "Read header failed: " << ec.message() << '\n';
                                     m_socket.close();
                                 }
                             });
        }

        void readBody() {
            asio::async_read(m_socket, asio::buffer(m_tempInMessage.body.data(), m_tempInMessage.body.size()),
                             [this](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     onMessage();
                                     readHeader();
                                     m_tempInMessage.body.clear();
                                 } else {
                                     std::cout << "Read body failed: " << ec.message() << '\n';
                                     m_socket.close();
                                 }
                             });
        }

        void onMessage() {
            owned_message<MsgTypes> message;
            if (m_owner == owner::server) {
                message.remote = this->shared_from_this();
            }

            message.message = m_tempInMessage;
            m_onMessageCallback(message);
        }

    };

}

#endif //FORTRESS_TCP_CONNECTION_H
