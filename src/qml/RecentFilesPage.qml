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
import QtQuick.Dialogs 1.2

Page {
    id: page

    actionBar.title: qsTr("Recent Files")
    actions: [
        Action {
            id: open
            iconName: "file/folder_open"
            name: qsTr("Open")
            onTriggered: openFileDialog.open()
        }
    ]

    FileDialog {
        id: openFileDialog
        title: qsTr("Choose a file to open")
        onAccepted: {
            pageStack.push(Qt.resolvedUrl("EditPage.qml"), { documentUrl: openFileDialog.fileUrl })
        }
    }

    FileGridView {
        id: recentFilesView
        model: sortedHistory
    }

    Scrollbar {
        flickableItem: recentFilesView
    }

    ActionButton {
        id: otherButton

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: Units.dp(24)

        iconName: "content/add"
    }
}
