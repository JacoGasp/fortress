import QtQuick 2.0
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 600
    height: 400
    title: {qsTr("Minimal Qml")}

    Text {
        anchors.fill: parent
        text: "Hello!"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 48
    }

    Gauge {
        objectName: "gauge"
    }

    Gauge {
        objectName: "gauge"
        x: 200
        y: 0
    }

    Gauge {
        objectName: "gauge"
        x: 400
        y: 0
    }

    Gauge {
        objectName: "gauge"
        x: 0
        y: 200
    }

    Gauge {
        objectName: "gauge"
        x: 200
        y: 200
    }

    Gauge {
        objectName: "gauge"
        x: 400
        y: 200
    }
}