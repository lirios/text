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
import QtGraphicalEffects 1.0
import Material 0.2

Flickable {
    id: rootFlickable

    property alias model: fileGrid.model
    property int margins: Units.dp(24)
    property int spacing: Units.dp(8)
    property int cardWidth: Units.dp(320)
    property int cardHeight: Units.dp(256)

    anchors.fill: parent
    contentHeight: fileGrid.height + spacing

    GridView {
        id: fileGrid

        y: spacing/2
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

                Rectangle {
                    color: "white"
                    clip: true
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: nameBackground.top

                    Text {
                        id: filePreview
                        anchors.fill: parent
                        anchors.margins: Units.dp(8)
                        anchors.rightMargin: Units.dp(4)
                        clip: true
                        font.family: "Roboto"
                        font.pixelSize: Units.dp(13)
                        font.weight: Font.Medium
                        text: previewText
                    }

                    LinearGradient {
                        anchors.fill: parent
                        start: Qt.point(parent.width - Units.dp(28), 0)
                        end: Qt.point(filePreview.width + filePreview.x, 0)
                        gradient: Gradient {
                            GradientStop {position: 0.0; color: "transparent"}
                            GradientStop {position: 1.0; color: "white"}
                        }
                    }
                }

                Rectangle {
                    id: nameBackground
                    color: "black"
                    opacity: 0.7
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: Units.dp(72)
                }

                Label {
                    id: docName

                    anchors.top: nameBackground.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: Units.dp(8)
                    anchors.leftMargin: Units.dp(16)
                    anchors.rightMargin: Units.dp(16)

                    text: name
                    color: "white"
                    font.pixelSize: Units.dp(20)
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }

                Label {
                    id: docUrl

                    property int symbolCount: (parent.width - Units.dp(16)) / Units.dp(8)

                    anchors.top: docName.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: Units.dp(4)
                    anchors.leftMargin: Units.dp(16)
                    anchors.rightMargin: Units.dp(16)

                    text: history.prettifyPath(fileUrl)
                    color: "white"
                    font.pixelSize: Units.dp(16)
                    font.weight: Font.Normal
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideMiddle
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
