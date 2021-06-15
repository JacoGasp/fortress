import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml

ToolBar {

    property bool bIpIsValid: false
    property bool bIsPortValid: true    // Workaround for default port
    property bool bIsPinging: false
    property bool bIsConnecting: false

    Connections {
        target: backend

        function onConnectionFailed(error_message) {
            statusLabel.text = `Status: Failed to connect - ${error_message}`
            statusIcon.color = "red"
            bIsConnecting = false
        }

        function onConnectionStatusChanged(bIsConnected) {
            console.log(bIsConnected ? "Connected to host" : "Disconnected from host")
            changeStatus(bIsConnected ? "connected" : "disconnected")
            bIsConnecting = false
        }
    }

    height: 150
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20

        ColumnLayout {
            RowLayout {
                GridLayout {
                    columns: 2
                    Label {
                        text: "IP Address:"
                        Layout.alignment: Qt.AlignRight
                    }

                    TextField {
                        Layout.minimumWidth: 120
                        id: ipAddressField
                        placeholderText: "192.168.1.7"
                        text: "127.0.0.1"

                        enabled: backend ? !(backend.bIsConnected || bIsConnecting) : false

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
                        enabled: backend ? !(backend.bIsConnected | bIsConnecting) : false
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
                        text: backend ? backend.bIsConnected ? "Disconnect" : "Connect" : "Disconnected"
                        enabled: bIpIsValid && bIsPortValid
                        onClicked: {
                            !backend.bIsConnected ? connect() : disconnect()
                        }
                    }

                    Button {
                        id: sendGreetings
                        text: "Greetings"
                        enabled: backend ? backend.bIsConnected : false
                        onClicked: {
                            backend.sendGreetings()
                        }
                    }
                }
            }
            RowLayout {
                Rectangle {
                    id: statusIcon
                    width: 10
                    height: 10

                    radius: width / 2
                    color: "red"
                    Label {
                        id: statusLabel
                        text: "Status: disconnected"
                        anchors.left: parent.right
                        anchors.leftMargin: 5
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

            }
        }

        ColumnLayout {
            GridLayout {
                columns: 2
                Button {
                    text: !bIsPinging ? "Ping" : "Stop"
                    enabled: backend ? backend.bIsConnected : false
                    onClicked: {
                        backend.togglePingUpdate();
                        bIsPinging = !bIsPinging;
                        bIsPinging ? start() : stop()
                    }
                }

                Label {
                    id: pingLabel
                    text: backend ? (backend.dPingValue >= Infinity ? "--" : `${backend.dPingValue.toFixed(1)} ms`) : "--"
                }

            }
        }
        ColumnLayout {
            Layout.fillWidth: true
        }
        ColumnLayout {
            GridLayout {
                columns: 2
                Label {
                    text: "Threshold"
                }
                TextField {
                    text: "10"
                    validator: DoubleValidator{
                        bottom: 0
                        top: 10000
                    }
                    onAccepted: {
                        root.threshold = this.text
                    }
                }

            }
        }

    }

    function start() {
        root.start()
    }


    function stop() {
        root.stop()
    }

    function connect() {
        bIsConnecting = true
        console.log("Attempting to connect...")
        changeStatus("connecting")

        backend.connectToHost(ipAddressField.text, portField.text)
    }

    function disconnect() {
        console.log("Disconnecting...")
        backend.disconnectFromHost()
        // Stop ping
        bIsPinging = false
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
