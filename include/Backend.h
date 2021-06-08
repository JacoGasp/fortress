//
// Created by Jacopo Gasparetto on 16/05/21.
//

#ifndef FORTRESS_BACKEND_H
#define FORTRESS_BACKEND_H

#include <QObject>
#include <QThreadPool>
#include <iostream>
#include <thread>
#include "Runnable.h"
#include "Networking/client.h"
#include "Constants.h"

using namespace fortress::net;

class Backend : public QObject, public ClientInterface<MsgTypes> {
Q_OBJECT
    Q_PROPERTY(double value READ getValue NOTIFY valueChanged)
    Q_PROPERTY(bool bIsConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(double dPingValue READ getLastPingValue NOTIFY pingReceived)

private:
    int m_t{};
    double m_value{};
    Runnable *m_runnable;

    double m_lastPingValue{ std::numeric_limits<double>::infinity() };
    bool m_isPinging { false };
    std::thread m_pingThread;
    static constexpr std::chrono::seconds PING_DELAY {1};

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

    [[nodiscard]] double getValue() const;

    [[nodiscard]] double getLastPingValue() const;

private:
    void pingHandler();

// Listen for events
public slots:

    void setNumber(int t);

// Emit signals
signals:

    void valueChanged(double);

    void connectionStatusChanged(bool bIsConnected);

    void connectionFailed(QString error_message);

    void pingReceived(double);

};

#endif //FORTRESS_BACKEND_H
