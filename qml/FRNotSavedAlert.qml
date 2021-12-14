import QtQuick 2.2
import Qt.labs.platform 1.1
import QtQuick.Controls

Item {
    parent: Overlay.overlay
    anchors.centerIn: parent
    MessageDialog {
        id: messageDialog
        text: "Current session was not saved"
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
