import QtQuick
import QtCharts

Rectangle {
    color: "#373A3C"

    ChartView {
        id: chartView
        anchors {fill: parent; margins: -12}
        antialiasing: true
        backgroundColor: "#55595C"

        legend.visible: false

        ValueAxis {
            id: axisX
            labelsColor: "lightgray"
            min: 0
            max: 1024
        }

        ValueAxis {
            id: axisY
            labelsColor: "lightgray"
            min: -10
            max: 5
        }

        LineSeries {
            id: lineSeries
            name: "LineSeries"
            axisX: axisX
            axisY: axisY
        }
    }

    Timer {
        id: refreshTimer
        interval: 1 / 60 * 1000
        running: true
        repeat: true
        onTriggered: {
            backend.updatePlotSeries(chartView.series(0))
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:1.75;height:480;width:640}
}
##^##*/

