//
// Created by Jacopo Gasparetto on 15/05/21.
//

#include <QApplication>
#include <QQmlEngine>
#include <QtQuick>
#include "Backend.h"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    auto f = engine.rootObjects().first()->findChildren<QObject *>("gauge");

    if (!f.empty())
        auto *gauge = f.first();

    Backend b{};
    return QApplication::exec();
}
