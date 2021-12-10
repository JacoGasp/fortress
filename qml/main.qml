//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQml

ApplicationWindow {
    id: root
    visible: true
    minimumWidth: SharedParams ? SharedParams.MIN_SCREEN_WIDTH : 0
    minimumHeight: SharedParams ? SharedParams.MAX_SCREEN_HEIGHT : 0
    title: {qsTr("Fortress")}
    color: "#373A3C"


    property bool isRunning: false
    readonly property int margins: 16
    property var charts: []
    property var gauges: []
    property double threshold: 1024
    property double thresholdIntegral: 100 * 1024
    property double ping: -1.0


    header: FRToolBar {
        id: headerId
    }


    Rectangle {
        id: background
        anchors.fill: parent
        color: "#373A3C"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        // Charts
        RowLayout {
            GridLayout {
                id: gridLayout
                columns: 4
                rows: 4

                //            Label {
                //                text: "Instant Dose"
                //                Layout.column: 3
                //                color: "lightGray"
                //            }

                //            Label {
                //                text: "Total Dose"
                //                Layout.column: 4
                //                color: "lightGray"
                //            }



                FRCharts {
                    id: chart_0
                    channel: 0
                    lineColor: "#D9534F"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1

                }

                FRCharts {
                    id: chart_1
                    channel:1
                    lineColor: "#56C0E0"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1

                }
                //            FRGauge {
                //                id: gauge_0
                //                channel: 0
                //                Layout.columnSpan: 1
                //                Layout.rowSpan: 1
                //                maxValue: threshold
                //            }

                //            FRGauge {
                //                id: gauge_0b
                //                channel: 0
                //                Layout.columnSpan: 1
                //                Layout.rowSpan: 1
                //                maxValue: thresholdIntegral
                //                bIsIntegral: true
                //            }

                FRCharts {
                    id: chart_2
                    channel: 2
                    lineColor: "#F0AD4E"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1
                }

                FRCharts {
                    id: chart_3
                    channel: 3
                    lineColor: "#5CB85C"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1
                }

                FRCharts {
                    id: chart_4
                    channel: 4
                    lineColor: "#D9534F"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1
                }

                FRCharts {
                    id: chart_5
                    channel: 5
                    lineColor: "#56C0E0"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1
                }

                FRCharts {
                    id: chart_6
                    channel: 6
                    lineColor: "#F0AD4E"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1
                }

                FRCharts {
                    id: chart_7
                    channel: 7
                    lineColor: "#5CB85C"
                    Layout.columnSpan: 2
                    Layout.rowSpan: 1
                }

                Layout.fillHeight: true

                Component.onCompleted: {

                    for (let i = 0; i < gridLayout.children.length; ++i)
                        charts.push(gridLayout.children[i]);
                }
            }
        }
        // Status Bar
        RowLayout {
            ColumnLayout{
                Text {
                    id: statusBar
                    text: ""
                    color: "lightgray"
                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                Text {
                    id: pingLabel
                    text: ping > 0 ? `Ping: ${ping.toFixed(3)} ms` : "Ping: -- ms"
                    color: "lightgray"
                }
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Timer {
        interval: 1000
        running: Backend ? Backend.bIsConnected : false
        repeat: true
        onTriggered: {
            ping = Backend.dPingValue
        }
    }

    function start() {
        gauges.forEach(g => g.start())
        charts.forEach(c => c.start())
    }

    function stop() {
        gauges.forEach(g => g.stop())
        charts.forEach(c => c.stop())
        ChartModel.clearData();
    }

    Connections {
        target: Backend
        function onConnectionStatusChanged(bIsConnected) {
            if (bIsConnected){
                Backend.togglePingUpdate()
            } else {
                ping = -1.0
            }
        }

        function onStatusBarMessageArrived(message) {
            statusBar.text = message
        }
    }
}


