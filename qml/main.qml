import QtQuick 2.0
import QtQuick.Controls 6.0
import Qt.labs.platform 1.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.2

import com.fortress.backend 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1200
    height: 840
    title: {qsTr("Fortress")}
    color: "#373A3C"

    property bool isRunning: false
    readonly property int margins: 16

    Backend {
        id: backend
        onPingReceived: {
            gauge1.value = this.dPingValue
        }

        onConnectionStatusChanged: {
            headerId.onConnectionStatusChanged(bIsConnected)
        }

        onConnectionFailed: {
            headerId.onConnectionFailed(error_message)
        }
    }

    MenuBar {
        id: menuBar
        Menu {
            id: fileMenu
            title: qsTr("File")
            // ...
        }

        Menu {
            id: editMenu
            title: qsTr("&Edit")
            // ...
        }

        Menu {
            id: viewMenu
            title: qsTr("&View")
            // ...
        }

        Menu {
            id: helpMenu
            title: qsTr("&Help")
            // ...
        }
    }

    header: FRToolBar {
        id: headerId
        backend: backend
    }


    Rectangle {
        id: background
        anchors.fill: parent
        color: "white"

        Gauge {
            id: gauge1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
