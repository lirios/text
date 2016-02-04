/*
 * Copyright © 2016 Andrew Penkrat
 *
 * This file is part of Liri Text.
 *
 * Liri Text is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Liri Text is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Liri Text.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.5
import Material 0.2
import QtQuick.Controls 1.4 as Controls
import me.liriproject.text 1.0

Page {
    id: page
    property url documentUrl

    actionBar.title: document.documentTitle
    actions: [
        Action {
            iconName: "content/save"
            name: qsTr("Save")
            onTriggered: {
                document.saveAs(documentUrl)
            }
        }
    ]

    Shortcut {
        sequence: "Ctrl+S"
        onActivated: document.saveAs(documentUrl)
    }

    Component.onCompleted: {
        history.touchFile(document.documentTitle, documentUrl)
    }

    Controls.TextArea {
        id: mainArea
        anchors.fill: parent
        focus: true
        font.family: "Roboto"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        text: document.text
    }

    DocumentHandler {
        id: document
        target: mainArea
        fileUrl: documentUrl
    }
}
