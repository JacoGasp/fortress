//
// SPDX-FileCopyrightText: 2021 INFN
// SPDX-License-Identifier: EUPL-1.2
// SPDX-FileContributor: Jacopo Gasparetto
//

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
