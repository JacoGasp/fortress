//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

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
            labelFormat: "%d"

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
            max: ChartModel ? ChartModel.plotWindowSize : 0
        }

        LineSeries {
            id: leftSeries
            name: ""
            axisX: axisX
            axisY: axisY
        }

        LineSeries {
            id: rightSeries
            name: "rightSeries"
            axisX: axisX
            axisY: axisY
        }
    }

    function start() {
        leftSeries.removePoints(0, leftSeries.count)
        rightSeries.removePoints(0, rightSeries.count)
        timer.start()
    }

    function stop() {
        timer.stop()
    }

    Timer {
        id: timer
        interval: 50
        running: isRunning
        repeat: true
        onTriggered: {
           update()
        }
    }

    function update() {
        let maxValue = ChartModel.getMaxChannelValue(channel)

        if (axisY.max !== maxValue)
            axisY.max = maxValue

        ChartModel.updatePlotSeries(leftSeries, rightSeries, channel)
    }

    Component.onCompleted: {
        leftSeries.color = lineColor
        rightSeries.color = lineColor
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:1.75;height:480;width:640}
}
##^##*/

