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
import QtQuick.Templates as T
// import SharedConstants

ToolBar {
    id: toolbar
    property bool bIpIsValid: false
    property bool bIsPortValid: true    // Workaround for default port
    property bool bIsConnecting: false
    property bool bIsReceiving: false
    property bool bIsSaveEnabled: false
    property bool bHasSaved: true
    property var startDate: new Date()

    FRNotSavedAlert {
        id: saveAlert
    }

    Connections {
        target: Backend

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

                    enabled: Backend ? !(Backend.bIsConnected || bIsConnecting) : false

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
                    enabled: Backend ? !(Backend.bIsConnected | bIsConnecting) : false
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
                    text: Backend ? Backend.bIsConnected ? "Disconnect" : "Connect" : "Disconnected"
                    enabled: bIpIsValid && bIsPortValid && !bIsConnecting && !bIsReceiving
                    onClicked: {
                        !Backend.bIsConnected ? connect() : disconnect()
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
                    enabled: Backend ? Backend.bIsConnected : false
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

            CheckBox {
                text: qsTr("Show differential values")
                checkState: Qt.Checked
                onCheckStateChanged: {
                    ChartModel.showDifferentialValues = this.checkState
                    console.log(`Show differential values: ${ChartModel.showDifferentialValues}`)
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
                    Layout.alignment: Qt.AlignRight
                }

                TextField {
                    text: SharedParams ? SharedParams.samplingFreq : 0
                    validator: IntValidator {
                        bottom: 3
                        top: SharedParams ? SharedParams.MAX_ALLOWED_FREQ : 0
                    }
                    enabled: !bIsReceiving
                    selectByMouse: true

                    onEditingFinished: {
                        SharedParams.samplingFreq = this.text
                    }

                    onFocusChanged: {
                        if(!focus && !acceptableInput)
                            this.text = SharedParams.samplingFreq
                    }

                    Layout.preferredWidth: 80
                }


                Label {
                    text: "HV (V):"
                    Layout.alignment: Qt.AlignRight
                }

                SpinBox {
                    id: spinbox
                    from: SharedParams ? SharedParams.kMinHVInMilliVolts : 0
                    value: SharedParams ? SharedParams.kDefaultHVInMilliVolts : 5000
                    to: SharedParams ? SharedParams.kMaxHVInMilliVolts : 0
                    stepSize: SharedParams ? SharedParams.kHVStepSizeInMilliVolts : 0
                    enabled: Backend ? Backend.bIsConnected && !bIsReceiving : false
                    editable: true

                    property int decimals: 3
                    property var roundToNearestStep: x => {
                        return SharedParams ?
                               Math.round(x / SharedParams.kHVStepSizeInMilliVolts) * SharedParams.kHVStepSizeInMilliVolts
                               : x
                    }
                    property real hv: roundToNearestStep(value)

                    validator: DoubleValidator {
                        bottom: Math.min(spinbox.from, spinbox.to)
                        top:  Math.max(spinbox.from, spinbox.to)
                    }

                    textFromValue: function(value, locale) {
                        return Number(roundToNearestStep(value) / 1000).toLocaleString(locale, 'f', spinbox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return roundToNearestStep(Number.fromLocaleString(locale, text) * 1000)
                    }

                    onFocusChanged: {
                        if(!focus)
                            spinbox.value = hv
                    }


                    Layout.preferredWidth: 80
                    Component.onCompleted: {
                        spinbox.contentItem.selectByMouse = true
                    }
                }
                Label{}
                Button {
                    text: "Send HV"
                    enabled: Backend ? Backend.bIsConnected && !bIsReceiving : false
                    Layout.preferredWidth: 80
                    onClicked: {
                        forceActiveFocus()
                        console.log(`Send HV value of ${spinbox.hv} mV`)
                        Backend.sendHVValue(spinbox.hv)
                    }
                }
            }
        }
    }


    FileDialog {
        id: fileDialog
        fileMode: FileDialog.SaveFile
        currentFile: `file:///${Qt.formatDate(startDate, "yyyyMMdd")}_${Qt.formatTime(startDate, "hhmmss")}_fortress.csv`
        onAccepted: {
            console.log(file)
            Backend.saveFile(file)
            bHasSaved = true
        }
    }

    function start() {
        startDate = new Date()
        if (!bHasSaved) {
            saveAlert.show()
        } else {
            bIsSaveEnabled = false
            bIsReceiving = true
            Backend.sendStartUpdateCommand(SharedParams.samplingFreq)
            root.start()
        }
    }


    function stop() {
        Backend.sendStopUpdateCommand()
        bIsSaveEnabled = true
        bIsReceiving = false
        bHasSaved = false
        root.stop()
    }

    function connect() {
        bIsConnecting = true
        console.log("Attempting to connect...")
        changeStatus("connecting")

        Backend.connectToHost(ipAddressField.text, portField.text)
    }

    function disconnect() {
        bIsSaveEnabled = false;
        console.log("Disconnecting...")
        Backend.disconnectFromHost()
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
