import QtQuick 2.0
import QtQuick.Controls 2.4

Rectangle {
    id: root
    width: 200
    height: 200
    color: "#40454d"

    readonly property var backgroundColor: "#58555A"
    readonly property var colorA: {hexToRgb("#2F9E69")}
    readonly property var colorB: {hexToRgb("#fcba03")}
    readonly property var colorC: {hexToRgb("#eb3434")}
    readonly property var lineWidth: 15
    readonly property var innerRadius: 15

    property var value
    property var currentColor: {Qt.rgba(0, 0, 0, 0)}

    onValueChanged: {
        currentColor = colorGrade(colorA, colorGrade(colorB, colorC))
        arc.requestPaint()
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

    function colorGrade (cA, cB) {

        var r = cA.r + value * (cB.r - cA.r)
        var g = cA.g + value * (cB.g - cA.g)
        var b = cA.b + value * (cB.b - cA.b)

        return Qt.rgba(r, g, b, 1)
    }

    Canvas {
        id: background
        anchors.fill: parent
        rotation: 135

        onPaint: {
            var c = getContext('2d');
            c.clearRect(0, 0, width, height)
            c.beginPath()
            c.lineWidth = lineWidth
            c.strokeStyle = backgroundColor
            c.arc(width / 2, height / 2, width / 2 - innerRadius, 0, Math.PI * 3/2)
            c.stroke()
        }
    }

    Canvas {
        id: arc
        anchors.fill: parent
        rotation: 135

        onPaint: {
            var c = getContext('2d');
            c.clearRect(0, 0, width, height)

            c.beginPath()
            c.lineWidth = lineWidth
            c.strokeStyle = currentColor
            c.globalAlpha = 1

            c.arc(width / 2, height / 2, width / 2 - innerRadius, 0, Math.PI * 3/2 * root.value)
            c.stroke()
        }
    }

    Text {
        id: textValue
        x: root.width / 2 - 25
        y: root.height / 2 - 25
        width: 50
        height: 50
        font.family: "Helvetica"
        font.pointSize: 48
        color: currentColor
        horizontalAlignment: Text.AlignHCenter
        text: {(value * 50.0).toFixed(1)}

        Text {
            x: 0
            y: textValue.height
            width: 50
            height: 50
            text: "mSv"
            font.pointSize: 18
            color: "lightgrey"
            horizontalAlignment: Text.AlignHCenter
        }
    }
}