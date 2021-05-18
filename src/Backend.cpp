//
// Created by Jacopo Gasparetto on 16/05/21.
//

#include "Backend.h"

Backend::Backend(QObject *parent)
    : QObject{parent} {
    m_runnable = new Runnable(this);
    m_runnable->setAutoDelete(false);
}

Backend::~Backend() {
    m_runnable->stop();
    delete m_runnable;
    std::cout << "runnable deleted, removing backend\n";
}

void Backend::startUpdate() {

    if (!m_runnable->isRunning()) {
        QThreadPool::globalInstance()->start(m_runnable);
        std::cout << "started thread pool\n";
    }
}

void Backend::stopUpdate() {
    if (m_runnable->isRunning()) {
        m_runnable->stop();
    }
}

double Backend::getValue() const {
    return m_value;
}

// Slots
void Backend::setNumber(int t) {
    if (t == m_t)
        return;

    m_t = t;
    m_value = (1.0 + std::sin(static_cast<double>(m_t) / 50.0)) / 2.0;
    emit valueChanged(m_value);
}
