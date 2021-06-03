//
// Created by Jacopo Gasparetto on 18/05/21.
//

#include "Runnable.h"

Runnable::Runnable(QObject *receiver) : m_receiver{ receiver } {}

void Runnable::run() {
    std::cout<< "Runnable at " << this << " started\n";
    m_isRunning = true;

    while (m_isRunning) {
        m_number++;
        QMetaObject::invokeMethod(m_receiver, "update", Qt::QueuedConnection);
    }
}

bool Runnable::isRunning() const {
    return m_isRunning;
}

void Runnable::stop() {
    m_isRunning = false;
    std::cout << "Runnable at " << this << " stopped\n";
}

std::ostream &operator<<(std::ostream &out, const Runnable &runnable) {
    out << "Runnable at " << &runnable;
    return out;
}
