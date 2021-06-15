import QtQuick
import QtQuick.Controls
import Qt.labs.platform

Item {
    MenuBar {
        id: menuBar
        Menu {
            id: fileMenu
            title: qsTr("File")
            // ...
            MenuItem {
                id: prefrences
                text: "Preferences"
            }
        }
    }
}
