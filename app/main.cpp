//
// Created by Jacopo Gasparetto on 15/05/21.
//

#include <QApplication>
#include <QQmlEngine>
#include <QtQuick>
#include "Backend.h"


int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication app(argc, argv);

    qmlRegisterType<Backend>("com.fortress.backend", 1, 0, "Backend");

    QQmlApplicationEngine engine;

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return QApplication::exec();
}