import QtQuick
import QtCharts

Rectangle {
    property int channel
    property string lineColor
    color: "#373A3C"

    ChartView {
        id: chartView
        anchors {fill: parent; margins: -12}
        antialiasing: true
        backgroundColor: "#55595C"
        legend.visible: false

        ValueAxis {
            id: axisY
            labelsColor: "lightgray"
            min: 0
            max: 10
            titleText: `<font color='lightgray'>Ch ${channel}</font>`
        }

        ValueAxis {
            id: axisX
            labelsColor: "lightgray"
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

