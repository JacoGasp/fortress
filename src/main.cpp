//
// Created by Jacopo Gasparetto on 15/05/21.
//

#include <iostream>
#include <QApplication>
#include <QQmlEngine>
#include <QtQuick>
#include <iostream>
#include <cmath>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    // Extract QObject


    // Bind to backend
//    Backend backend{ gauge };

    return QApplication::exec();
}
