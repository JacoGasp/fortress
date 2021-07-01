import QtQuick
import QtCharts
import QtQuick.Layouts

Rectangle {
    property int channel
    property string lineColor
    color: "#373A3C"

    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.preferredHeight: Layout.rowSpan
    Layout.preferredWidth: Layout.columnSpan


    ChartView {
        id: chartView
        anchors {fill: parent; margins: -12}
        antialiasing: true
        backgroundColor: "#55595C"
        legend.visible: false

        ValueAxis {
            id: axisY
            labelsColor: "darkgray"
            min: 0
            max: 10
            titleText: `<font color='lightgray'>Ch ${channel}</font>`
            gridLineColor: "darkgray"

            Behavior on min {
                NumberAnimation { duration: 200 }
            }

            Behavior on max {
                NumberAnimation { duration: 200 }
            }

        }

        ValueAxis {
            id: axisX
            labelsColor: "darkgray"
            visible: false
            min: 0
            max: backend ? backend.windowSize : 0
        }

        LineSeries {
            id: lineSeries
            name: "LineSeries"
            axisX: axisX
            axisY: axisY
        }

        LineSeries {
            id: lineSeriesOld
            name: "LineSeriesOld"
            axisX: axisX
            axisY: axisY
        }
    }

    function update() {
        let maxValue = backend.getMaxChannelValue(channel)

        if (axisY.max !== maxValue)
            axisY.max = maxValue

        backend.updatePlotSeries(lineSeries, lineSeriesOld, channel)
    }

    Component.onCompleted: {
        lineSeries.color = lineColor
        lineSeriesOld.color = lineColor
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:1.75;height:480;width:640}
}
##^##*/

