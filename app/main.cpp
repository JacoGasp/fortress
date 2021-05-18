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
//    Backend b;
//    b.startUpdate();

    qmlRegisterType<Backend>("com.fortress.backend", 1, 0, "Backend");

    QQmlApplicationEngine engine;
//    engine.rootContext()->setContextProperty("backend", &b);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return QApplication::exec();
}