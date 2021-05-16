import QtQuick 2.0
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: {qsTr("Minimal Qml")}

    Text {
        anchors.fill: parent
        text: "Hello!"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 48
    }
}