//
// Created by Jacopo Gasparetto on 16/05/21.
//

#ifndef FORTRESS_BACKEND_H
#define FORTRESS_BACKEND_H

#include <QtCore/QtMath>
#include <QtCore/QObject>
#include <QtCore/QRandomGenerator>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QtQuick/QQuickView>
#include <iostream>
#include <thread>
#include <QFile>
#include <QDir>
#include "Networking/client.h"
#include "Constants.h"

using namespace fortress::net;

class Backend : public QObject, public ClientInterface<MsgTypes> {
Q_OBJECT
    Q_PROPERTY(bool bIsConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(double dPingValue READ getLastPingValue NOTIFY pingReceived)
    Q_PROPERTY(int windowSize READ windowSize() NOTIFY windowSizeChanged)
    Q_PROPERTY(QList<QPointF> series READ getSeries())

private:
    double m_lastPingValue{ std::numeric_limits<double>::infinity() };
    bool m_isPinging{ false };
    std::thread m_pingThread;
    static constexpr std::chrono::milliseconds PING_DELAY{ 100 };

    int m_data_idx{ -1 };                                                  // X axis index
    QList<QList<QPointF>> m_data;                                          // (nChannel x windowSize) data to display
    std::array<double, fortress::consts::N_CHANNELS> m_chLastValues{};     // Store temporary last reads per each channel for plotting
    std::array<double, fortress::consts::N_CHANNELS> m_chMaxValues{};      // Store temporary maxValue per each channel for autoscaling plot
    std::array<double, fortress::consts::N_CHANNELS> m_chIntegralValues{}; // Store total cumulative values

    QFile m_file;
    QTextStream m_textStream{&m_file};

public:
    // Avoid name collision with multiple inheritance
    using ClientInterface<MsgTypes>::connect;

    explicit Backend(QObject *parent = nullptr);

    ~Backend() override;

    Q_INVOKABLE bool connectToHost(const QString &host, uint16_t port);

    Q_INVOKABLE void disconnectFromHost();

    Q_INVOKABLE void sendGreetings();

    Q_INVOKABLE void togglePingUpdate();

    void onMessage(message<MsgTypes> &msg) override;

    void onConnectionFailed(std::error_code &ec) override;

    // Accessors

    [[nodiscard]] double getLastPingValue() const;

    [[nodiscard]] QList<QPointF> getSeries() const;

    [[nodiscard]] int windowSize() const;


private:
    void pingHandler();

    void onReadingsReceived(message<MsgTypes> &msg);

    void openFile();

    void closeFile();

// Listen for events
public slots:

    void generatePlotSeries(int n_channels, int length);

    void addPointsToSeries(const std::array<double, fortress::consts::N_CHANNELS> &values);

    void updatePlotSeries(QAbstractSeries *newSeries, QAbstractSeries *oldSeries, int channel);

    [[nodiscard]] double getLastChannelValue(int channel) const;

    [[nodiscard]] double getMaxChannelValue(int channel) const;

    [[nodiscard]] double getIntegralChannelValue(int channel) const;

    void sendStartUpdateCommand();

    void sendStopUpdateCommand();


// Emit signals
signals:

    void connectionStatusChanged(bool bIsConnected);

    void connectionFailed(QString error_message);

    void pingReceived(double ping);

    void readingsReceived();

    void windowSizeChanged();

};

#endif //FORTRESS_BACKEND_H
