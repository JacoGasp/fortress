import QtQuick
import QtQuick.Controls

Rectangle {
    id: gauge
    width: 150
    height: 150
    color: "#55595C"
    radius: 5

    readonly property string backgroundColor: "#817D85"
    readonly property var colorA: {hexToRgb("#2F9E69")}
    readonly property var colorB: {hexToRgb("#fcba03")}
    readonly property var colorC: {hexToRgb("#eb3434")}
    readonly property int lineWidth: 15
    readonly property int innerRadius: 15

    readonly property real maxValue: 50
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

    function colorGrade(cA, cB) {

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

        onPaint: {
            var c = getContext('2d')
            c.clearRect(0, 0, width, height)

            c.beginPath()
            c.lineWidth = lineWidth
            c.strokeStyle = currentColor
            c.globalAlpha = 1

            c.arc(width / 2, height / 2, width / 2 - innerRadius, 0,
                  Math.PI * 3 / 2 * gauge.value / maxValue)
            c.stroke()
        }
    }

    Text {
        id: textValue
        anchors.centerIn: parent
        font.family: "Helvetica"
        font.pointSize: 48
        color: isNaN(value) ? backgroundColor : currentColor
        horizontalAlignment: Text.AlignHCenter
        //        text: { (value * maxValue).toFixed(1) }
        text: isNaN(value) ? "--" : value.toFixed(1)
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 80
            text: "mSv"
            font.pointSize: 18
            color: "lightgrey"
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
