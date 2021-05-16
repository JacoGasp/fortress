//
// Created by Jacopo Gasparetto on 15/05/21.
//

#include <QApplication>
#include <QQmlEngine>
#include <QtQuick>
#include <iostream>
#include "Backend.h"


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    auto f = engine.rootObjects().first()->findChildren<QObject *>("gauge");

    Backend b;

    if (!f.empty()) {
        auto *gauge = f.first();
        std::cout << "Found a gauge! " << gauge << '\n';

        b.addGauge(gauge);
        b.startUpdate();
    }

    return QApplication::exec();
}