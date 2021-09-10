#include "TCPServer.h"

TCPServer::TCPServer(uint16_t port) : m_server(port) {
    m_server.onClient([&](void *arg, AsyncClient *c) {
        c->onData([&](void *arg, AsyncClient *client, void *data, size_t len) {
            onData(arg, client, data, len);
        });
    }, this);
}

void TCPServer::onConnect(void *arg, AsyncClient *client) {
    std::cout << "connect!" << '\n';
}

void TCPServer::begin() {
    m_server.begin();
}

void TCPServer::end() {
    m_server.end();
}

void TCPServer::printData(uint8_t *data, size_t len) {
    for (auto ptr{data}; ptr != data + len; ++ptr)
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)(*ptr) << ' ';    
    std::cout << std::endl;
}

void TCPServer::readHeader(uint8_t *data) {
    std::memcpy(&m_tempInMessage.header, data, sizeof(Header));

    if (m_tempInMessage.header.size > 0){
        // Make space to fit the incoming body
        m_tempInMessage.body.resize(m_tempInMessage.header.size);
        // Tell the server that it should expect a body
        m_expectedBodyLength = m_tempInMessage.header.size;
    }
}

void TCPServer::readBody(uint8_t *data) {
    std::memcpy(m_tempInMessage.body.data(), data, m_tempInMessage.size());
}

void TCPServer::onMessage(const Message &msg, AsyncClient *client) {
    std::cout << msg << std::endl;
    
    // Make an action according to the Header ID
    switch (msg.header.id) {
    case fortress::net::MsgTypes::ServerPing: {
        sendMessage(msg, client);
    }
        break;
    
    default:
        break;
    }
}

// Here we can have data of any length: smaller, greater or equal to the full message (header + body), 
// so we have to keep track of the amout of data we already stored
void TCPServer::onData(void *arg, AsyncClient *client, void *data, size_t len) {
    // Cast the pointer in oreder to have a valid arithmetic
    uint8_t *buffer = static_cast<uint8_t*>(data);
    size_t offset = 0;
    // Keep processing data until we have processed all the valid messages in the queue
    while (len - offset > 0) {
        // We are ready to read the header
        if (m_expectedBodyLength == 0) {
            readHeader(buffer + offset);
            offset += sizeof(Header);
        } 
        // We already read the header and we expect a not-null body
        else {
            readBody(buffer + offset);
            offset += m_tempInMessage.size();
            // We read the body 
            m_expectedBodyLength = 0;
        }

        // There is no more data to process and we have a valid message
        if (m_expectedBodyLength == 0) {
            onMessage(m_tempInMessage, client);
        }
    }
}

void TCPServer::sendMessage(const Message &msg, AsyncClient *client) {
    std::array<char, sizeof(Header)> header;
    std::memcpy(header.data(), &msg.header, sizeof(Header));

    client->add(header.data(), sizeof(Header), ASYNC_WRITE_FLAG_COPY);
    client->add(reinterpret_cast<const char*>(msg.body.data()), msg.size(), ASYNC_WRITE_FLAG_COPY);
    client->send();
}