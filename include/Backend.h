//
// Created by Jacopo Gasparetto on 16/05/21.
//

#ifndef FORTRESS_BACKEND_H
#define FORTRESS_BACKEND_H

#include <QObject>
#include <QThreadPool>
#include <iostream>
#include "Runnable.h"

class Backend : public QObject {
Q_OBJECT
    Q_PROPERTY(double value READ getValue NOTIFY valueChanged)

private:
    int m_t{};
    double m_value{};
    Runnable *m_runnable;

public:
    explicit Backend(QObject *parent = nullptr);

    ~Backend() override;

    Q_INVOKABLE void startUpdate();

    Q_INVOKABLE void stopUpdate();

    [[nodiscard]] double getValue() const;


// Listen for events
public slots:

    void setNumber(int t);

// Emit signals
signals:
    void valueChanged(double);

};

#endif //FORTRESS_BACKEND_H
