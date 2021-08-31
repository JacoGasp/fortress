//
// Created by Jacopo Gasparetto on 10/08/21.
//

#ifndef FORTRESS_CONNECTIONS_V2_H
#define FORTRESS_CONNECTIONS_V2_H

#include <utility>

#include "commons.h"
#include "message.h"
#include "Constants.h"

namespace fortress::net {

    class ConnectionV2 {

    protected:
        // This context is shared with the whole asio instance
        asio::io_context &m_asioContext;

        // Each connection has a unique socket to a remote
        asio::ip::tcp::socket m_socket;
        message<MsgTypes> m_tempInMessage;
        std::function<void(message<MsgTypes>&)> m_onMessageCallback;

    public:
        ConnectionV2(asio::io_context &asioContext, asio::ip::tcp::socket socket,
                     std::function<void(message<MsgTypes>&)> callback) :
                m_asioContext{ asioContext },
                m_socket{ std::move(socket) },
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
                                        m_socket.close();
                                    }
                                });
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
        void send(message<MsgTypes> &msg) {
            asio::post(m_asioContext, [this, msg]() {
                writeHeader(msg);
            });
        }

    private:
        void writeHeader(const message<MsgTypes> &msg) {
            asio::async_write(m_socket, asio::buffer(&msg.header, sizeof(MessageHeader<MsgTypes>)),
                              [this, msg](std::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      if (!msg.body.empty())
                                          writeBody(msg);
                                      return;
                                  }
                                  std::cout << "Write header failed: " << ec.message() << "\n";
                              });
        }

        void writeBody(const message<MsgTypes> &msg) {
            asio::async_write(m_socket, asio::buffer(msg.body.data(), msg.body.size()),
                              [this](std::error_code ec, std::size_t length) {
                                  if (ec) {
                                      std::cout << "Write body failed: " << ec.message() << '\n';
                                      m_socket.close();
                                  }
                              });
        }


        void readHeader() {
            asio::async_read(m_socket, asio::buffer(&m_tempInMessage.header, sizeof(MessageHeader<MsgTypes>)),
                             [this](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     if (m_tempInMessage.header.size > 0) {
                                         m_tempInMessage.body.resize(m_tempInMessage.header.size);
                                         readBody();
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
                                     readHeader();
                                 } else {
                                     std::cout << "Read body failed: " << ec.message() << '\n';
                                     m_socket.close();
                                 }

                             });
        }

    };

}

#endif //FORTRESS_CONNECTIONS_V2_H
