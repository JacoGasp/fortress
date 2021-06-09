import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQml

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

    Connections {
        target: backend

        function onPingReceived(ping) {
            gauge1.value = ping
        }
    }

}


