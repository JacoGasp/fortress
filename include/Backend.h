//
// Created by Jacopo Gasparetto on 16/05/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#ifndef FORTRESS_BACKEND_H
#define FORTRESS_BACKEND_H

#include <QTemporaryFile>
#include <iostream>
#include <QFile>
#include <QDir>
#include "networking/client_interface.h"
#include "constants.h"
#include "SharedParams.h"
#include "ChartModel.h"

using namespace fortress::net;

class Backend : public QObject, public client_interface {
Q_OBJECT
    Q_PROPERTY(bool bIsConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(double dPingValue READ getLastPingValue())
    Q_PROPERTY(QString statusBarMessage READ getStatusBarMessage NOTIFY statusBarMessageArrived)

private:
    ChartModel *m_chartModel;

    double m_lastPingValue{ std::numeric_limits<double>::infinity() };
    bool m_bIsPinging{ false };
    static constexpr asio::chrono::milliseconds PING_DELAY{ 1000 };

    QTemporaryFile m_file{ "fortress_out.csv" };              // Csv output file
    QTextStream m_textStream{ &m_file };                             // Csv stream to write on file

    asio::io_context m_context{};
    std::unique_ptr<asio::steady_timer> m_pPingTimer;
    std::thread m_threadContext;

    std::chrono::time_point<std::chrono::steady_clock> m_startUpdateTime;

    unsigned long m_readingsReceived{ 0 };
    unsigned long m_bytesRead{ 0 };
    unsigned long m_prevReadingTimestamp{ 0 };
    QString m_statusBarMessage{};
    bool m_askDisconnect = false;

public:
    // Avoid name collision with multiple inheritance
    using client_interface::connect;

    explicit Backend(ChartModel *chartModel, QObject *parent = nullptr);

    ~Backend() override;

    void onServerDisconnected() override;

    Q_INVOKABLE void connectToHost(const QString &host, uint16_t port);

    Q_INVOKABLE void disconnectFromHost();

    Q_INVOKABLE void togglePingUpdate();

    Q_INVOKABLE void sendStartUpdateCommand(uint16_t frequency);

    Q_INVOKABLE void sendStopUpdateCommand();

    Q_INVOKABLE void sendHVValue(uint16_t value);

    Q_INVOKABLE bool saveFile(QUrl &destination_path);

    void onMessage(message<MsgTypes> &msg) override;

    // Accessors

    [[nodiscard]] double getLastPingValue() const;

    [[nodiscard]] QString getStatusBarMessage() const;


private:
    void pingHandler();

    void onReadingsReceived(message<MsgTypes> &msg);

    void onServerFinishedUpload();

    void openFile(uint16_t frequency);

    void closeFile();

// Emit signals
signals:

    void connectionStatusChanged(bool bIsConnected);

    void connectionLost();

    void statusBarMessageArrived(QString message);

    void connectionFailed(QString error_message);

    void pingReceived(double ping);

};

#endif //FORTRESS_BACKEND_H
