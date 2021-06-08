//
// Created by Jacopo Gasparetto on 16/05/21.
//

#include "Backend.h"

Backend::Backend(QObject *parent)
    : QObject{parent}{
    m_runnable = new Runnable(this);
    m_runnable->setAutoDelete(false);
    std::cout << "Instantiated backend helper\n";
}

Backend::~Backend() {
    m_runnable->stop();
    delete m_runnable;
    std::cout << "runnable deleted, removing backend\n";
    disconnectFromHost();
}

double Backend::getValue() const {
    return m_value;
}

bool Backend::connectToHost(const QString &host, uint16_t port) {
    startListening();
    return connect(host.toStdString(), port);
}

void Backend::disconnectFromHost() {
    if (isConnected()) {
        message<MsgTypes> disconnectMsg;
        disconnectMsg.header.id = ClientDisconnect;
        send(disconnectMsg);
    }

    stopListening();

    if (m_isPinging)
        togglePingUpdate();

    bool prevIsConnected {isConnected()};
    ClientInterface<MsgTypes>::disconnect();
    if (prevIsConnected != isConnected())
        emit connectionStatusChanged(isConnected());

    if (!isConnected()) {
        std::cout << "[BACKEND]: disconnected from host" << std::endl;
    }


}

void Backend::sendGreetings() {
    message<MsgTypes> msg;
    msg.header.id = ClientMessage;
    msg << "John";
    send(msg);
}

void Backend::onConnectionFailed(std::error_code &ec) {
    stopListening();
    emit connectionFailed(QString::fromStdString(ec.message()));
}

void Backend::onMessage(message<MsgTypes> &msg) {
    switch (msg.header.id) {
        case MsgTypes::ServerAccept: {
            std::cout << "Server accepted\n";
            emit connectionStatusChanged(true);
            break;
        }

        case MsgTypes::ServerMessage: {
            std::cout << "Message from server: " << msg << ":\n";
            std::ostringstream out;
            for (auto ch: msg.body) {
                out << ch;
            }
            std::cout << out.str() << std::endl;
        }
            break;

        case MsgTypes::ServerPing: {
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point timeThen;
            msg >> timeThen;
            m_lastPingValue = std::chrono::duration<double>(timeNow - timeThen).count() * 1000;

            std::cout << "Ping: " << m_lastPingValue << " ms.\n";

            emit pingReceived(m_lastPingValue);
        }
            break;

        case MsgTypes::ClientPing:
            send(msg);
            break;

        default:
            std::cout << "???\n";
    }
}

// Helpers
void Backend::pingHandler() {
    std::cout << "[BACKEND] Ping thread started\n";
    while (m_isPinging) {
        message<MsgTypes> pingMsg;
        pingMsg.header.id = ServerPing;
        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
        pingMsg << timeNow;
        send(pingMsg);
        std::this_thread::sleep_for(PING_DELAY);
    }
    std::cout << "[BACKEND] Ping thread stopped\n";
}

void Backend::togglePingUpdate() {
    if (m_isPinging) {
        m_isPinging = false;
        m_pingThread.detach();
    } else {
        m_isPinging = true;
        m_pingThread = std::thread{&Backend::pingHandler, this};
    }
}

double Backend::getLastPingValue() const {
    return m_lastPingValue;
}

// Slots
void Backend::setNumber(int t) {
    if (t == m_t)
        return;

    m_t = t;
    m_value = (1.0 + std::sin(static_cast<double>(m_t) / 50.0)) / 2.0;
    emit valueChanged(m_value);
}
