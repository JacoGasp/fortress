import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 1.11

ToolBar {

    property bool bIpIsValid: false
    property bool bIsPortValid: true
    property bool bIsPinging: false
    property var backend

    height: 150
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20

        ColumnLayout {
            RowLayout {
                GridLayout {
                    columns: 2
                    Label {
                        text: "Ip Address:"
                        Layout.alignment: Qt.AlignRight
                    }

                    TextField {
                        id: ipAddressField
                        placeholderText: "192.168.1.7"
                        text: "127.0.0.1"
                        Layout.minimumWidth: 120
                        validator: RegularExpressionValidator {
                            regularExpression: /^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])/
                        }
                        selectByMouse: true
                        onTextChanged: {
                            bIpIsValid = acceptableInput
                        }
                        Component.onCompleted: {
                            bIpIsValid = acceptableInput
                        }
                    }

                    Label {
                        text: "Port:"
                        Layout.alignment: Qt.AlignRight
                    }

                    TextField {
                        id: portField
                        text: "60000"
                        Layout.maximumWidth: 60
                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }
                        selectByMouse: true
                        onTextChanged: {
                            bIsPortValid = acceptableInput
                        }
                    }

                    rowSpacing: 10

                    Button {
                        id: connectButton
                        Layout.preferredWidth: 100
                        text: backend.bIsConnected ? "Disconnect" : "Connect"
                        enabled: bIpIsValid && bIsPortValid
                        onClicked: {
                            if (!backend.bIsConnected) {
                                console.log("Attempting to connect...")
                                changeStatus("connecting")
                                backend.connectToHost(ipAddressField.text,
                                                      portField.text)
                                backend.bIsConnected ? changeStatus("connected") : changeStatus("disconnected")
                                console.log("Connection successfull")
                                backend.startUpdate()
                            } else {
                                console.log("Disconnecting...")
                                backend.disconnectFromHost()
                                backend.bIsConnected ? changeStatus("connected") : changeStatus("disconnected")

                                // Stop ping
                                bIsPinging = false
                            }
                        }
                    }

                    Button {
                        id: sendGreetings
                        text: "Greetings"
                        enabled: backend.bIsConnected
                        onClicked: {
                            backend.sendGreetings()
                        }
                    }
                }
            }
            RowLayout {
                Label {
                    id: statusLabel
                    text: "Status: disconnected"
                    Rectangle {
                        id: statusIcon
                        width: 10
                        height: 10
                        y: parent.height / 2 - height / 2
                        anchors.left: parent.right
                        anchors.leftMargin: 2
                        radius: width / 2
                        color: "red"
                    }
                }
            }
        }

        ColumnLayout {
            GridLayout {
                columns: 2
                Button {
                    text: !bIsPinging ? "Ping" : "Stop"
                    enabled: backend.bIsConnected
                    onClicked: {
                        backend.togglePingUpdate();
                        bIsPinging = !bIsPinging;
                    }
                }
                Label {
                    text: `${backend.dPingValue.toFixed(1)} ms`;
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
        }

    }


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
            statusLabel.text = "Status: connected"
            statusIcon.color = "green"
            break
        }
    }
}
