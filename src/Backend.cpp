//
// Created by Jacopo Gasparetto on 16/05/21.
//

#include "Backend.h"
#include <iostream>
#include <cmath>

Backend::Backend(QObject *parent)
        : QObject(parent) {

    m_timer = new QTimer(this);
    m_gauge = new QObject(this);
    connect(m_timer, &QTimer::timeout, this, &Backend::update);
}


Backend::~Backend() {
    m_timer = nullptr;
    m_gauge = nullptr;
    delete m_timer;
    delete m_gauge;
}


void Backend::startUpdate() {
    m_timer->start(10);
}

void Backend::addGauge(QObject *gauge) {
    m_gauge = gauge;
    std::cout << "gauge address: " << gauge << '\n';
}

void Backend::update() {
    static int t{};
    double value = (1.0 + std::sin(static_cast<double>(t) / 50.0)) / 2.0;
    m_gauge->setProperty("value", value);
    ++t;
}