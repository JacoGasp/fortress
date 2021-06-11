import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQml

ApplicationWindow {
    id: root
    visible: true
    width: 1150
    height: 840
    title: {qsTr("Fortress")}
    color: "#373A3C"

    property bool isRunning: false
    readonly property int margins: 16


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
        anchors.fill: parent
        anchors.margins: 5
        columns: 2

        FRCharts {
            id: chart_0
            channel: 0
            lineColor: "#D9534F"
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.fillWidth: true
        }

        Gauge {
            id: gauge_0
            Layout.preferredHeight: 150
        }

        FRCharts {
            id: chart_1
            channel: 1
            lineColor: "#F0AD4E"
            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        Gauge {
            id: gauge_1
            Layout.preferredHeight: 150
        }

        FRCharts {
            id: chart_2
            channel: 2
            lineColor: "#5CB85C"
            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        Gauge {
            id: gauge_2
            Layout.preferredHeight: 150
        }

        FRCharts {
            id: chart_3
            channel: 3
            lineColor: "#56C0E0"
            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        Gauge {
            id: gauge_3
            Layout.preferredHeight: 150
        }
    }

    Connections {
        target: backend
        function onPingReceived() {
            chart_0.update()
            chart_1.update()
            chart_2.update()
            chart_3.update()

            gauge_0.value = backend.getLastChannelValue(0)
            gauge_1.value = backend.getLastChannelValue(1)
            gauge_2.value = backend.getLastChannelValue(2)
            gauge_3.value = backend.getLastChannelValue(3)
        }
    }
}


