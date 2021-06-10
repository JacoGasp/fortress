//
// Created by Jacopo Gasparetto on 16/05/21.
//

#include "Backend.h"

Backend::Backend(QObject *parent)
        : QObject{ parent } {
    generatePlotSeries(1, 1024);
    std::cout << "Instantiated backend helper\n";
}

Backend::~Backend() {
    disconnectFromHost();
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

    bool prevIsConnected{ isConnected() };
    ClientInterface<MsgTypes>::disconnect();

    if (prevIsConnected != isConnected())
            emit connectionStatusChanged(isConnected());

    if (!isConnected()) {
        std::cout << "[BACKEND] Disconnected from host" << std::endl;
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
            std::cout << "[BACKEND] Server accepted\n";
            emit connectionStatusChanged(true);
            break;
        }

        case MsgTypes::ServerMessage: {
            std::cout << "[BACKEND] Message from server: " << msg << ": ";
            std::ostringstream out;

            for (auto ch: msg.body)
                out << ch;

            std::cout << out.str() << std::endl;
            break;
        }

        case MsgTypes::ServerPing: {
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point timeThen;

            msg >> timeThen;

            m_lastPingValue = std::chrono::duration<double>(timeNow - timeThen).count() * 1000;
            std::cout << "Ping: " << m_lastPingValue << " ms.\n";

            emit pingReceived(m_lastPingValue);

            break;
        }

        case MsgTypes::ClientPing: {
            send(msg);
            break;
        }

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
        m_pingThread = std::thread{ &Backend::pingHandler, this };
    }
}

double Backend::getLastPingValue() const {
    return m_lastPingValue;
}

// Slots
void Backend::generatePlotSeries(int n_channels, int length) {
    m_data.clear();
    for (int ch = 0; ch < n_channels; ++ch) {
        QList<QPointF> points;
        points.reserve(length);
        for (int i = 0; i < length; ++i) {
            double x{ static_cast<double>(i) };
            double y{ qSin(M_PI / 50.0 * i) + 0.5 + QRandomGenerator::global()->generateDouble() };
            points.append(QPointF(x, y));
        }
        m_data.append(points);
    }
}

void Backend::updatePlotSeries(QAbstractSeries *series) {
    if (series) {
        auto *xySeries = dynamic_cast<QXYSeries *>(series);
        m_data_idx++;
        if (m_data_idx > m_data.count() - 1)
            m_data_idx = 0;

        generatePlotSeries(1, 1024);
//        QList<QPointF> points = m_data.at(m_data_idx);
        QList<QPointF> points = m_data.at(0); // First series
        xySeries->replace(points);
    }
}

QList<QPointF> Backend::getSeries() const {
    return m_data.at(0);
}