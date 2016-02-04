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

Flickable {
    id: rootFlickable

    property alias model: fileGrid.model
    property int margins: Units.dp(24)
    property int spacing: Units.dp(16)
    property int cardWidth: Units.gu(5)
    property int cardHeight: Units.gu(4)

    anchors.fill: parent
    contentHeight: fileGrid.height

    GridView {
        id: fileGrid

        width: ~~((parent.width - 2*margins + spacing) / (cardWidth+spacing)) * (cardWidth+spacing)
        anchors.horizontalCenter: parent.horizontalCenter
        height: childrenRect.height
        cellWidth: cardWidth + spacing
        cellHeight: cardHeight + spacing

        delegate: Item {
            width: fileGrid.cellWidth
            height: fileGrid.cellHeight

            Card {
                id: fileCard
                anchors.fill: parent
                anchors.margins: spacing / 2

                Text {
                    anchors.fill: parent
                    anchors.margins: Units.dp(8)
                    font.family: "Roboto"
                    text: previewText
                }

                Rectangle {
                    id: nameBackground
                    color: "black"
                    opacity: 0.5
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: Units.dp(72)
                }

                Label {
                    id: docName

                    anchors.top: nameBackground.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: Units.dp(8)

                    text: name
                    color: "white"
                    font.pixelSize: Units.dp(20)
                    font.weight: Font.Medium
                }

                Label {
                    id: docUrl

                    anchors.top: docName.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: Units.dp(4)

                    text: humanReadableUrl
                    color: "white"
                    font.pixelSize: Units.dp(16)
                    font.weight: Font.Normal
                }

                Ink {
                    id: animation
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: {
                        if(mouse.button == Qt.LeftButton)
                            pageStack.push(Qt.resolvedUrl("EditPage.qml"), {documentUrl: fileUrl})
                        else
                            // Rightclicking deletes item for debugging
                            history.removeFile(fileUrl)
                    }
                }
            }
        }
    }
}
