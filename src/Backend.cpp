//
// Created by Jacopo Gasparetto on 16/05/21.
//

#include "Backend.h"

Backend::Backend(QObject *parent)
        : QObject{ parent } {
    generatePlotSeries(4, m_windowSizeInPoint);
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
            // std::cout << "Ping: " << m_lastPingValue << " ms.\n";

            m_chLastValues[0] = m_lastPingValue;
            m_chLastValues[1] = 2 + m_lastPingValue / 0.7;
            m_chLastValues[2] = 8 - m_lastPingValue;
            m_chLastValues[3] = 4 + m_lastPingValue * .5;

            for (int i = 0; i < m_nChannels; ++i)
                if (m_chLastValues[i] > m_chMaxValues[i]) m_chMaxValues[i] = m_chLastValues[i];

            addPointsToSeries(m_chLastValues);
            emit pingReceived(m_lastPingValue);

            break;
        }

        case MsgTypes::ClientPing: {
            send(msg);
            break;
        }

        case MsgTypes::ServerReadings: {
            onReadingsReceived(msg);
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

void Backend::onReadingsReceived(message<MsgTypes> &msg) {
    msg >> m_chLastValues;

    for (int i = 0; i < m_nChannels; ++i)
        if (m_chLastValues[i] > m_chMaxValues[i]) m_chMaxValues[i] = m_chLastValues[i];

    addPointsToSeries(m_chLastValues);
    emit readingsReceived();
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

    for (int ch = 0; ch < n_channels; ++ch) {
        QList<QPointF> points{};
        points.reserve(length);

        for (int i = 0; i < length; ++i)
            points.push_front(QPointF{});

        m_data.append(points);
    }
}

void Backend::addPointsToSeries(const std::array<double, m_nChannels> &values) {

    static int t{ 0 };
    m_data_idx = t % m_windowSizeInPoint;
    for (int ch = 0; ch < m_nChannels; ++ch) {

        auto chSeries = &m_data[ch];

        double x{ static_cast<double>(m_data_idx % (m_windowSizeInPoint)) };
        double y{ values[ch] };
        chSeries->replace(m_data_idx, QPointF{ x, y });

        auto max = *std::max_element(chSeries->begin(), chSeries->end(), [](const QPointF &p1, const QPointF &p2) {
            return p1.y() < p2.y();
        });

        m_chMaxValues[ch] = max.y();
        m_chIntegralValues[ch] += values[ch];
    }
    ++t;
}

void Backend::updatePlotSeries(QAbstractSeries *newSeries, QAbstractSeries *oldSeries, int channel) {
    if (newSeries && oldSeries) {
        auto *xyNewSeries = dynamic_cast<QXYSeries *>(newSeries);
        auto *xyOldSeries = dynamic_cast<QXYSeries *>(oldSeries);

        if (m_data_idx == 0) {
            xyNewSeries->removePoints(0, xyNewSeries->count());
            xyOldSeries->replace(m_data[channel]);
        } else {
            xyOldSeries->remove(0);
        }

        xyNewSeries->append(m_data[channel][m_data_idx]);
    }
}

// Accessors

QList<QPointF> Backend::getSeries() const {
    return m_data.at(0);
}

int Backend::windowSize() const {
    return m_windowSizeInPoint;
}

void Backend::setWindowSize(int windowSize) {
    m_windowSizeInPoint = windowSize;
}

double Backend::getLastChannelValue(int channel) const {
    return m_chLastValues[channel];
}

double Backend::getMaxChannelValue(int channel) const {
    return m_chMaxValues[channel];
}

double Backend::getIntegralChannelValue(int channel) const {
    return m_chIntegralValues[channel];
}

void Backend::sendStartUpdateCommand() {
    message<MsgTypes> msg;
    msg.header.id = ClientStartUpdating;
    send(msg);
}

void Backend::sendStopUpdateCommand() {
    message<MsgTypes> msg;
    msg.header.id = ClientStopUpdating;
    send(msg);
}
