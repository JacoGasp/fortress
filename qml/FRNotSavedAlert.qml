import QtQuick 2.2
import Qt.labs.platform 1.1
import QtQuick.Controls

Item {
    parent: Overlay.overlay
    anchors.centerIn: parent
    MessageDialog {
        id: messageDialog
        text: "Starting a new session will overwrite the current one."
        informativeText: "Do you want to save?"
        buttons: StandardButton.No | StandardButton.Save

        onAccepted: {
            fileDialog.open()
        }

        onRejected: {
            toolbar.bHasSaved = true
            toolbar.start()
        }
    }

    function show() {
        messageDialog.open()
    }
}
