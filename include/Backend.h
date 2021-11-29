//
// Created by Jacopo Gasparetto on 16/05/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#ifndef FORTRESS_BACKEND_H
#define FORTRESS_BACKEND_H

#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QtQuick/QQuickView>
#include <QTemporaryFile>
#include <QtCore/QObject>
#include <QtCore/QtMath>
#include <iostream>
#include <QFile>
#include <QDir>
#include "networking/client_interface.h"
#include "constants.h"

using namespace fortress::net;

class Backend : public QObject, public client_interface {
Q_OBJECT
    Q_PROPERTY(bool bIsConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(double dPingValue READ getLastPingValue())
    Q_PROPERTY(int windowSize READ windowSize() NOTIFY windowSizeChanged)

private:
    double m_lastPingValue{ std::numeric_limits<double>::infinity() };
    bool m_bIsPinging{ false };
    static constexpr asio::chrono::milliseconds PING_DELAY{ 1000 };

    int m_data_idx{ 0 };                                                   // X axis index
    int m_t{ 0 };
    QList<QList<QPointF>> m_data;                                          // (nChannel x windowSize) data to display
    std::array<uint16_t , fortress::consts::N_CHANNELS> m_chLastValues{};     // Store temporary last reads per each channel for plotting
    std::array<uint16_t, fortress::consts::N_CHANNELS> m_chMaxValues{};      // Store temporary maxValue per each channel for autoscaling plot
    std::array<double, fortress::consts::N_CHANNELS> m_chIntegralValues{}; // Store total cumulative values

    QTemporaryFile m_file{"fortress_out.csv"};              // Csv output file
    QTextStream m_textStream{&m_file};                             // Csv stream to write on file

    asio::io_context m_context{};
    std::unique_ptr<asio::steady_timer> m_pPingTimer;
    std::thread m_threadContext;

    std::chrono::time_point<std::chrono::steady_clock> m_startUpdateTime;

    unsigned long m_readingsReceived{0};
    unsigned long m_bytesRead{0};

public:
    // Avoid name collision with multiple inheritance
    using client_interface::connect;

    explicit Backend(QObject *parent = nullptr);

    ~Backend() override;

    void onServerDisconnected() override;

    Q_INVOKABLE bool connectToHost(const QString &host, uint16_t port);

    Q_INVOKABLE void disconnectFromHost();

    Q_INVOKABLE void togglePingUpdate();

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void sendStartUpdateCommand(uint16_t frequency);

    Q_INVOKABLE void sendStopUpdateCommand();

    Q_INVOKABLE void saveFile(QUrl &destination_path);


    void onMessage(message<MsgTypes> &msg) override;

//    void onConnectionFailed(std::error_code &ec) override;

    // Accessors

    [[nodiscard]] double getLastPingValue() const;

    [[nodiscard]] QList<QPointF> getSeries() const;

    [[nodiscard]] static int windowSize() ;


private:
    void pingHandler();

    void onReadingsReceived(message<MsgTypes> &msg);

    void openFile();

    void closeFile();

// Listen for events
public slots:

    void generatePlotSeries(int n_channels, int length);

    void addPointsToSeries(const std::array<uint16_t, 4> &values);

    void updatePlotSeries(QAbstractSeries *qtQuickLeftSeries, QAbstractSeries *qtQuickRightSeries, int channel);

    [[nodiscard]] double getLastChannelValue(int channel) const;

    [[nodiscard]] double getMaxChannelValue(int channel) const;

    [[nodiscard]] double getIntegralChannelValue(uint8_t channel) const;


// Emit signals
signals:

    void connectionStatusChanged(bool bIsConnected);

    void connectionFailed(QString error_message);

    void pingReceived(double ping);

    void windowSizeChanged();

};

#endif //FORTRESS_BACKEND_H
