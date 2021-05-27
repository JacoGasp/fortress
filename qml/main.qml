import QtQuick 2.0
import QtQuick.Controls 6.0
import Qt.labs.platform 1.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.2

import com.fortress.backend 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 1200
    height: 840
    title: {qsTr("Minimal Qml")}
    color: "#373A3C"

    property bool isRunning: false
    readonly property int margins: 16

    function start() {
        window.isRunning = true
        console.log("start")
        backend.startUpdate()

    }

    function stop() {
        isRunning = false
        console.log("stop")
        backend.stopUpdate()
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

    }


    Rectangle {
        id: menuBarRec
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: margins
        height: 50
        color: "#55595C"
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: "white"
    }

    Button {
        id: button
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        x: 300
        y: 400 - 80
        text: {!isRunning ? "start" : "stop"}
        onClicked: {!isRunning ? start() : stop()}
    }


    Gauge {
        id: gauge1
        anchors.top: menuBar.bottom
        anchors.right: parent.right
        anchors.margins: margins

    }

    Backend {
        id: backend
        onValueChanged: {
            gauge1.value = this.value
        }
    }
}
