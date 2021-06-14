import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQml

ApplicationWindow {
    id: root
    visible: true
    minimumWidth: 1280
    minimumHeight: 800
    title: {qsTr("Fortress")}
    color: "#373A3C"


    property bool isRunning: false
    readonly property int margins: 16
    readonly property int nChannels: 4
    property var charts: []
    property var gauges: []

    MenuBar {
        id: menuBar
        Menu {
            id: fileMenu
            title: qsTr("File")
            // ...
        }

        Menu {
            id: editMenu
            title: qsTr("&Edit")
            // ...
        }

        Menu {
            id: viewMenu
            title: qsTr("&View")
            // ...
        }

        Menu {
            id: helpMenu
            title: qsTr("&Help")
            // ...
        }
    }

    header: FRToolBar {
        id: headerId
    }


    Rectangle {
        id: background
        anchors.fill: parent
        color: "#373A3C"
    }

    GridLayout {
        id: gridLayout
        anchors.fill: parent
        anchors.margins: 5
        columns: 4
        rows: 4

        FRCharts {
            id: chart_0
            channel: 0
            lineColor: "#D9534F"
            Layout.columnSpan: 3
            Layout.rowSpan: 1

        }

        FRGauge {
            id: gauge_0
            channel: 0
            Layout.columnSpan: 1
            Layout.rowSpan: 1
        }

        FRCharts {
            id: chart_1
            channel: 1
            lineColor: "#F0AD4E"
            Layout.columnSpan: 3
            Layout.rowSpan: 1
        }

        FRGauge {
            id: gauge_1
            channel: 1
            Layout.columnSpan: 1
            Layout.rowSpan: 1
        }

        FRCharts {
            id: chart_2
            channel: 2
            lineColor: "#5CB85C"
            Layout.columnSpan: 3
            Layout.rowSpan: 1
        }

        FRGauge {
            id: gauge_2
            channel: 2
            Layout.columnSpan: 1
            Layout.rowSpan: 1
        }

        FRCharts {
            id: chart_3
            channel: 3
            lineColor: "#56C0E0"
            Layout.columnSpan: 3
            Layout.rowSpan: 1
        }

        FRGauge {
            id: gauge_3
            channel: 3
            Layout.columnSpan: 1
            Layout.rowSpan: 1
        }

        Layout.fillHeight: true

        Component.onCompleted: {
            charts = [chart_0, chart_1, chart_2, chart_3]
            gauges = [gauge_0, gauge_1, gauge_2, gauge_3]
        }
    }

    Connections {
        target: backend
        function onPingReceived() {
            charts.forEach(c => c.update())
        }
    }

    function start() {
        gauges.forEach(g => g.start())
    }

    function stop() {
        gauges.forEach(g => g.stop())
    }
}


