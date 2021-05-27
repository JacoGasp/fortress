import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 1.11

ToolBar {

    property bool bIpIsValid: false
    property bool bIsPortValid: true


    height: 150
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20

        GridLayout {
            columns: 2
            Label {
                text: "Ip Address:"
                Layout.alignment: Qt.AlignRight
            }

            TextField {
                placeholderText: "192.168.1.7"
                Layout.minimumWidth: 120
                validator: RegularExpressionValidator { regularExpression: /^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])/}
                selectByMouse: true
                onTextChanged: { bIpIsValid = acceptableInput }
            }

            Label {
                text: "Port:"
                Layout.alignment: Qt.AlignRight
            }

            TextField {
                text: "60000"
                Layout.maximumWidth: 60
                validator: IntValidator { bottom: 1; top: 65535}
                selectByMouse: true
                onTextChanged: { bIsPortValid = acceptableInput }
            }

            rowSpacing: 10

            Button {
                id: connectButton
                text: "Connect"
                enabled: bIpIsValid && bIsPortValid
                onClicked: { changeStatus("connecting") }
            }

            Label {
                id: statusLabel
                text: "Status: disconnected"
                Rectangle {
                    id: statusIcon
                    width: 10
                    height: 10
                    y: parent.height / 2 - height / 2
                    anchors.right: parent.right
                    anchors.rightMargin: -20
                    radius: width / 2
                    color: "red"
                }
            }
        }

    }
    ColumnLayout{Layout.fillWidth: true}

    function changeStatus(status) {
        switch (status) {
        case "disconnected":
            statusLabel.text = "Status: disconnected"
            statusIcon.color = "red"
            break
        case "connecting":
            statusLabel.text = "Status: connecting"
            statusIcon.color = "orange"
            break
        case "connected":
            statusLabel = "Status: connected"
            statusIcon.color = "green"
            break
        }
    }
}
