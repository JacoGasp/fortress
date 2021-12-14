//
// Created by Jacopo Gasparetto on 16/05/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#include "Backend.h"

Backend::Backend(ChartModel *chartModel, QObject *parent)
        :
        QObject{ parent },
        client_interface{ m_context },
        m_chartModel{ chartModel },
        m_pPingTimer{ std::make_unique<asio::steady_timer>(m_context, PING_DELAY) } {

    // m_file.setAutoRemove(true);
    std::cout << "Instantiated backend helper\n";
}

Backend::~Backend() {
    m_pPingTimer->cancel();
    disconnectFromHost();

    // Join any dangling thread before exit
    if (m_threadContext.joinable())
        m_threadContext.join();

    closeFile();
    std::cout << "[BACKEND] Closing. Bye.\n";
}


void Backend::connectToHost(const QString &host, uint16_t port) {
    m_askDisconnect = false;
    // Prepare the context for consecutive use
    m_context.restart();

    // Join any dangling contextThread
    if (m_threadContext.joinable())
        m_threadContext.join();

    // This returns immediately
    connect(host.toStdString(), port);

    // Start the threadContext
    m_threadContext = std::thread([&]() {
        m_context.run();
        assert(m_context.stopped() == true);
        std::cout << "[BACKEND] Asio context stopped\n";
        emit connectionStatusChanged(isConnected());

        if (!m_askDisconnect) {
            std::cerr << "[BACKEND] Connection to ESP 32 lost\n";
            emit connectionLost();
        }
    });
}

void Backend::disconnectFromHost() {
    m_askDisconnect = true;
    if (!client_interface::isConnected())
        return;

    std::cout << "[BACKEND] Disconnecting... \n";

    message<MsgTypes> disconnectMsg;
    disconnectMsg.header.id = ClientDisconnect;
    sendMessage(disconnectMsg);

    client_interface::disconnect();
}

void Backend::onServerDisconnected() {
    std::cout << "[BACKEND] Connection dropped, disconnect.\n";
    assert(!client_interface::isConnected());

    if (m_bIsPinging)
        togglePingUpdate();

    if (!m_context.stopped()) {
        std::cout << "[BACKEND] Stopping Asio context\n";
        m_context.stop();
    }

    emit statusBarMessageArrived("Connection closed.");
}

void Backend::onMessage(message<MsgTypes> &msg) {
    switch (msg.header.id) {
        case MsgTypes::ServerAccept: {
            std::cout << "[BACKEND] Server accepted\n";
            emit connectionStatusChanged(true);
            emit statusBarMessageArrived("ESP32 accepted connection");
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
            break;
        }

        case MsgTypes::ClientPing: {
            sendMessage(msg);
            break;
        }

        case MsgTypes::ServerReadings: {
            onReadingsReceived(msg);
            break;
        }

        case MsgTypes::ServerFinishedUpload: {
            std::cout << "[BACKEND] Server finished upload\n";
            onServerFinishedUpload();
            break;
        }

        default:
            std::cout << "???\n";
    }
}

// Helpers

void Backend::onReadingsReceived(message<MsgTypes> &msg) {
    try {
        // Get channels values
        uint32_t time;
        uint32_t deltaTime;
        CurrentReadings_t currentReadings{};

        msg >> time;
        deltaTime = time - m_prevReadingTimestamp;

        for (int i = 0; i < SharedParams::n_channels; ++i) {
            // Note: channels are flipped in respect of ESP 32 order
            uint16_t newReading;
            msg >> newReading;
            auto lastReading = m_ADCReadings[i];

            // The integrator has been reset.
            if (lastReading - newReading > SharedParams::integratorThreshold * 0.9)
                lastReading -= SharedParams::integratorThreshold;

            // Compute current in Ampere
            currentReadings[i] = computeCurrentFromADC(newReading, lastReading, deltaTime);
            m_ADCReadings[i] = newReading;
        }

        // Count the amount of data received
        m_bytesRead += sizeof(msg);
        ++m_readingsReceived;

        // Write data to disk
        m_textStream << time << ',' << deltaTime;
        for (int i = 0; i < SharedParams::n_channels; ++i) {
            m_textStream << ',' << m_ADCReadings[i] << ',' << currentReadings[i];
        }
        m_textStream << '\n';

        // Draw
        m_chartModel->insertReadings(m_ADCReadings, currentReadings);

        m_prevReadingTimestamp = time;
    } catch (std::exception const &e) {
        std::cout << "Caught exception parsing new reading: " << e.what() << '\n';
    } catch (...) {
        std::cout << "Caught unknown exception parsing new reading\n";
    }
}

void Backend::onServerFinishedUpload() {
    // How long the session was
    std::chrono::duration<double> elapsedTime = std::chrono::steady_clock::now() - m_startUpdateTime;

    auto kilobytes = m_bytesRead / 1000.0;

    std::stringstream report;
    report << "Received " << m_readingsReceived + 1 << " readings. Transferred "
      << kilobytes << " KB in " << elapsedTime.count() << " s - "
      << kilobytes / elapsedTime.count() << " KB/s";

    emit statusBarMessageArrived(QString::fromStdString(report.str()));
    std::cout << report.str() << std::endl;
}

void Backend::pingHandler() {
    if (m_bIsPinging) {
        message<MsgTypes> pingMsg;
        pingMsg.header.id = ServerPing;
        pingMsg << std::chrono::system_clock::now();

        sendMessage(pingMsg);

        m_pPingTimer->expires_from_now(PING_DELAY);
        m_pPingTimer->async_wait([this](asio::error_code ec) {
            pingHandler();
        });
    }
}

void Backend::togglePingUpdate() {
    if (m_bIsPinging) {
        m_bIsPinging = false;
        std::cout << "[BACKEND] Stopped ping updates\n";
    } else {
        m_bIsPinging = true;
        pingHandler();
        std::cout << "[BACKEND] Start ping updates\n";
    }
}

void Backend::openFile(uint16_t frequency) {
    m_file.open();
    m_file.seek(0);
    m_textStream.flush();

    auto now = QDateTime::currentDateTime();
    auto offset = now.offsetFromUtc();
    now.setOffsetFromUtc(offset);

    m_textStream << "############ Fortress ############" << '\n'
                 << "Timestamp: " << now.toString(Qt::ISODate) << '\n'
                 << "Sampling Frequency (Hz): " << frequency << '\n'
                 << "ADC Max Value: " << SharedParams::kADCMaxVal << '\n'
                 << "ADC Vref (V): " << SharedParams::kADCVref << '\n'
                 << "Amplifier Feedback: " << SharedParams::kAmplifierFeedback << '\n'
                 << "Integrator Capacitance (pF): " << SharedParams::kIntegratorCapacitance << '\n'
                 << "##################################" << '\n'
                 << "t" << ",delta_t";

    for (int i = 0; i < SharedParams::n_channels; ++i)
        m_textStream << ",ADC_ch_" << i << ",I_ch_" << i;
    m_textStream << Qt::endl;
}

void Backend::closeFile() {
    m_textStream.flush();

    if (m_file.isOpen())
        m_file.close();
}

double Backend::getLastPingValue() const {
    return m_lastPingValue;
}

QString Backend::getStatusBarMessage() const {
    return m_statusBarMessage;
}

// Accessors

void Backend::sendStartUpdateCommand(uint16_t frequency) {
    openFile(frequency);
    // Clear the status bar
    emit statusBarMessageArrived("");

    message<MsgTypes> msg;
    msg.header.id = ClientStartUpdating;
    msg << frequency;
    m_startUpdateTime = std::chrono::steady_clock::now();
    m_readingsReceived = 0;
    m_bytesRead = 0;
    m_ADCReadings = {};
    sendMessage(msg);
}

void Backend::sendStopUpdateCommand() {

    message<MsgTypes> msg;
    msg.header.id = ClientStopUpdating;
    sendMessage(msg);
}


void Backend::sendHVValue(uint16_t value) {
    message<MsgTypes> msg;
    msg.header.id = fortress::net::ClientSetSensorHV;
    msg << value;
    sendMessage(msg);
}

bool Backend::saveFile(QUrl &destinationPath) {
    closeFile();
    if (QFile::exists(destinationPath.path())) {
        std::cout << "Destination " << destinationPath.path().toStdString() << " already exists, overwrite.";
        QFile::remove(destinationPath.path());
    }
    return QFile::copy(m_file.fileName(), destinationPath.path());
}