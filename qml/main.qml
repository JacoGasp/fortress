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
//    title: {qsTr("Fortress")}
//    color: "#373A3C"

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
//        backend: backend
    }


    Rectangle {
        id: background
        anchors.fill: parent
        color: "#373A3C"
    }

    Connections {
        target: backend

        function onPingReceived(ping) {
            gauge1.value = ping
        }
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 5
        columns: 2

        FRCharts {
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.fillWidth: true
        }

        Gauge {
            id: gauge1
            Layout.preferredHeight: 150
        }

        FRCharts {
            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        Gauge {
            Layout.preferredHeight: 150
        }

        FRCharts {
            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        Gauge {
            Layout.preferredHeight: 150
        }

        FRCharts {
            Layout.preferredHeight: 150
            Layout.fillWidth: true
        }

        Gauge {
            Layout.preferredHeight: 150
        }
    }

}


