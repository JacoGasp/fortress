//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: gauge
    color: "#55595C"
    radius: 5

    property int channel: 0

    readonly property string backgroundColor: "#817D85"
    readonly property var colorA: hexToRgb("#2F9E69")
    readonly property var colorB: hexToRgb("#fcba03")
    readonly property var colorC: hexToRgb("#eb3434")
    readonly property int lineWidth: 15
    readonly property int innerRadius: 15

    property real maxValue: 10
    property var currentColor: {Qt.rgba(0, 0, 0, 0)}
    property bool isRunning: false
    property bool bIsIntegral: false

    Layout.fillHeight: true
    Layout.minimumWidth: 150
    Layout.minimumHeight: 150

    onHeightChanged: {
        Layout.preferredWidth = height
    }


    function start() {
        isRunning = true
    }

    function stop() {
        isRunning = false
    }

    function hexToRgb(hex) {
        var result = /([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})/i.exec(hex)
        if (result)
            return Qt.rgba(
                parseInt(result[1], 16) / 255,
                parseInt(result[2], 16) / 255,
                parseInt(result[3], 16) / 255,
            )
    }

    function colorGrade(cA, cB, value) {

        var r = cA.r + value / maxValue * (cB.r - cA.r)
        var g = cA.g + value / maxValue * (cB.g - cA.g)
        var b = cA.b + value / maxValue * (cB.b - cA.b)

        return Qt.rgba(r, g, b, 1)
    }

    Canvas {
        id: background
        anchors.fill: parent
        rotation: 135

        onPaint: {
            var c = getContext('2d')
            c.clearRect(0, 0, width, height)
            c.beginPath()
            c.lineWidth = lineWidth
            c.strokeStyle = backgroundColor
            c.arc(width / 2, height / 2, width / 2 - innerRadius, 0,
                  Math.PI * 3 / 2)
            c.stroke()
        }
    }

    Canvas {
        id: arc
        anchors.fill: parent
        rotation: 135
        property real value
        property string color: colorA

        onValueChanged: {
            requestPaint()
        }

        onPaint: {
            var c = getContext('2d')

            if (value > maxValue)
                value = maxValue

            let maxAngle = Math.PI * 3 / 2
            let angle =  maxAngle * value / maxValue

            c.clearRect(0, 0, width, height)

            c.beginPath()
            c.lineWidth = lineWidth
            c.strokeStyle = color
            c.globalAlpha = 1

            c.arc(width / 2, height / 2, width / 2 - innerRadius, 0, angle > maxAngle ? maxAngle : angle)
            c.stroke()
        }

        Behavior on value {
            NumberAnimation { duration: 200 }
        }

        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }

    Text {
        id: textValue
        anchors.centerIn: parent
        font.family: "Helvetica"
        font.pointSize: 32
        horizontalAlignment: Text.AlignHCenter
        color: backgroundColor
        text: "--"

        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: textValue.bottom
        // text: bIsIntegral ? "Sv" : "mSv"
        text: "ADC"
        font.pointSize: 18
        color: "lightgrey"
        horizontalAlignment: Text.AlignHCenter
    }

    Timer {
        interval: 1000
        running: isRunning
        repeat: true
        onTriggered: {
           update(bIsIntegral ? ChartModel.getChannelTotalSum(channel) / 1000 : ChartModel.getLastChannelValue(channel))
        }
    }

    function update(newValue) {

        textValue.text = newValue.toFixed(0)

        if (newValue > maxValue)
            newValue = maxValue

        let newColor = colorGrade(colorA, colorGrade(colorB, colorC, newValue), newValue)

        textValue.color = newColor
        arc.color = newColor
        arc.value = newValue
    }
}
