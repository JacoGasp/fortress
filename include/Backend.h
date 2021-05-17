//
// Created by Jacopo Gasparetto on 16/05/21.
//

#ifndef FORTRESS_BACKEND_H
#define FORTRESS_BACKEND_H

#include <QtCore>
#include <iostream>

class Backend : public QObject {
    Q_OBJECT

private:
    QPointer<QTimer> m_timer;
    QVector<QObject*> m_gauges;

public:
    explicit Backend(QObject *parent = nullptr);

    ~Backend() override;

    void addGauge(QObject *gauge);

    void startUpdate();

public slots:
    void update();
};

#endif //FORTRESS_BACKEND_H
