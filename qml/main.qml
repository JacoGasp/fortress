import QtQuick 2.0
import QtQuick.Controls

import com.fortress.backend 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 600
    height: 400
    title: {qsTr("Minimal Qml")}

    property var isRunning: false

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


    Text {
        id: hello
        anchors.fill: parent
        text: "Hello!"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 48
    }

    Gauge {
        id: gauge1
        x: 0
        y: 0
    }

    Backend {
        id: backend
        onValueChanged: {
            gauge1.value = this.value
        }
    }
}
