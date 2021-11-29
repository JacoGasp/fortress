//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import Qt.labs.platform
// import SharedConstants

ToolBar {
    id: toolbar
    property bool bIpIsValid: false
    property bool bIsPortValid: true    // Workaround for default port
    property bool bIsConnecting: false
    property bool bIsReceiving: false
    property bool bIsSaveEnabled: false
    property bool bHasSaved: true

    FRNotSavedAlert {
        id: saveAlert
    }

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
            bIsReceiving = false
        }
    }

    height: 150
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20

        ColumnLayout {

            GridLayout {
                columns: 2
                Label {
                    text: "IP Address:"
                    Layout.alignment: Qt.AlignRight
                }

                TextField {
                    Layout.minimumWidth: 120
                    id: ipAddressField
                    placeholderText: SharedParams ? SharedParams.IP_PLACEHOLDER : ""
                    text: SharedParams ?  SharedParams.ip : ""

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
                    text: SharedParams ? SharedParams.defaultPort : 0
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

                Label{}
                Button {
                    id: connectButton
                    text: backend ? backend.bIsConnected ? "Disconnect" : "Connect" : "Disconnected"
                    enabled: bIpIsValid && bIsPortValid && !bIsConnecting && !bIsReceiving
                    onClicked: {
                        !backend.bIsConnected ? connect() : disconnect()
                    }
                }


                Rectangle {
                    id: statusIcon
                    width: 10
                    height: 10

                    radius: width / 2
                    color: "red"
                    Label {
                        id: statusLabel
                        text: "Status: Disconnected"
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
                    text: !bIsReceiving ? "Start" : "Stop"
                    enabled: backend ? backend.bIsConnected : false
                    onClicked: {
                        !bIsReceiving ? start() : stop()
                    }
                }

                Button {
                    text: "Save"
                    enabled: bIsSaveEnabled && !bHasSaved
                    onClicked: {
                        fileDialog.open()
                    }
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
                    text: "Sampl. Freq (Hz):"
                }

                TextField {
                    text: SharedParams ? SharedParams.samplingFreq : 0
                    validator: IntValidator {
                        bottom: 3
                        top: SharedParams ? SharedParams.MAX_ALLOWED_FREQ : 0
                    }
                    enabled: !bIsReceiving

                    onEditingFinished: {
                        SharedParams.samplingFreq = this.text
                    }

                    onFocusChanged: {
                        if(!focus && !acceptableInput)
                            this.text = SharedParams.samplingFreq
                    }

                    Layout.preferredWidth: 50
                }

                Label {
                    text: "Threshold (mSv):"
                }

                TextField {
                    text: root.threshold
                    validator: IntValidator {
                        bottom: 1
                        top: SharedParams ? SharedParams.MAX_ALLOWED_FREQ : 1
                    }

                    onEditingFinished: {
                        root.threshold = this.text
                    }

                    onFocusChanged: {
                        if(!focus && !acceptableInput)
                            this.text = root.threshold
                    }
                    Layout.preferredWidth: 50
                }
            }
        }
    }


    FileDialog {
        id: fileDialog
        fileMode: FileDialog.SaveFile
        onAccepted: {
            console.log(file)
            backend.saveFile(file)
            bHasSaved = true
        }
    }

    function start() {
        if (!bHasSaved) {
            saveAlert.show()
        } else {
            bIsSaveEnabled = false
            bIsReceiving = true
            backend.sendStartUpdateCommand(SharedParams.samplingFreq)
            root.start()
        }
    }


    function stop() {
        backend.sendStopUpdateCommand()
        bIsSaveEnabled = true
        bIsReceiving = false
        bHasSaved = false
        root.stop()
    }

    function connect() {
        bIsConnecting = true
        console.log("Attempting to connect...")
        changeStatus("connecting")

        backend.connectToHost(ipAddressField.text, portField.text)
    }

    function disconnect() {
        bIsSaveEnabled = false;
        console.log("Disconnecting...")
        backend.disconnectFromHost()
    }


    function changeStatus(status) {
        switch (status) {
        case "disconnected":
            statusLabel.text = "Status: Disconnected"
            statusIcon.color = "red"
            break
        case "connecting":
            statusLabel.text = "Status: Connecting..."
            statusIcon.color = "orange"
            break
        case "connected":
            statusLabel.text = "Status: Connected"
            statusIcon.color = "green"
            break
        }
    }
}
