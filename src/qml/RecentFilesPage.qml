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

import QtQuick 2.4
import Material 0.2
import QtQuick.Controls 1.4 as Controls
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
            pageStack.push(Qt.resolvedUrl("EditPage.qml"), { filename: openFileDialog.fileUrl })
        }
    }

    GridView {
        id: recentFilesView

        anchors.fill: parent
        model: 1
        delegate: Item {
            width: recentFilesView.width / 2
            height: width

            Card {
                id: newFile
                anchors.fill: parent
                anchors.margins: Units.dp(14)

                Label {
                    id: docName
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: Units.dp(14)

                    text: "New Document.txt"
                    font.pixelSize: Units.dp(24)
                }

//                Icon {
//                    id: plusIcon
//                    anchors.centerIn: parent
//                    size: parent.width * 2/3
//                    name: "content/add"
//                }

                Ink {
                    id: animation
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    onClicked: {
                        pageStack.push(Qt.resolvedUrl("EditPage.qml"), {filename: "file://home/andrew/New Document.txt"})
                    }
                }
            }
        }
    }
}
