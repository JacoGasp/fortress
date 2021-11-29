//
// Created by Jacopo Gasparetto on 15/05/21.
//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

#include <QApplication>
#include <QQmlEngine>
#include <QtQuick>
#include <QtQml>
#include "Backend.h"
#include "SharedParams.h"


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    bool openGLSupported = QQuickWindow::graphicsApi() == QSGRendererInterface::OpenGLRhi;
    if (!openGLSupported) {
        qWarning() << "OpenGL is not set as the graphics backend, so AbstractSeries.useOpenGL will not work.";
        qWarning() << "Set QSG_RHI_BACKEND=opengl environment variable to force the OpenGL backend to be used.";
    }

    QQmlApplicationEngine engine;

    Backend backend;
    engine.rootContext()->setContextProperty("backend", &backend);

    SharedParams sharedParams;
    engine.rootContext()->setContextProperty("SharedParams", &sharedParams);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return QApplication::exec();
}