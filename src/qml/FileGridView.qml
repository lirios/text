/*
 * Copyright © 2016-2017 Andrew Penkrat
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
import Fluid.Controls 1.0
import Fluid.Material 1.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

Flickable {
    id: rootFlickable

    property alias model: fileGridContents.model
    property int margins: 24
    property int spacing: 8
    property int cardWidth: 240
    property int cardHeight: 194

    anchors.fill: parent
    contentHeight: fileGrid.height + spacing

    ScrollBar.vertical: ScrollBar { }

    GridLayout {
        id: fileGrid

        y: spacing/2
        columns: ~~((parent.width - 2*margins + spacing) / (cardWidth+spacing))
        width: columns * (cardWidth+spacing)
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            id: fileGridContents

            delegate: Card {
                id: fileCard

                contentWidth: cardWidth
                contentHeight: cardHeight
                Layout.preferredWidth: cardWidth + spacing
                Layout.preferredHeight: cardHeight + spacing

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
                        anchors.margins: 8
                        anchors.rightMargin: 4
                        clip: true
                        font.family: defaultFont.family
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        textFormat: Text.RichText
                        text: previewText
                    }

                    LinearGradient {
                        anchors.fill: parent
                        start: Qt.point(parent.width - 28, 0)
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
                    height: 72
                }

                Label {
                    id: docName

                    anchors.top: nameBackground.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 8
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16

                    text: name
                    color: "white"
                    font.pixelSize: 20
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }

                Label {
                    id: docUrl

                    property int symbolCount: (parent.width - 16) / 8

                    anchors.top: docName.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.topMargin: 4
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16

                    text: history.prettifyPath(fileUrl)
                    color: "white"
                    font.pixelSize: 16
                    font.weight: Font.Normal
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideMiddle
                }

                Ripple {
                    id: animation
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onClicked: {
                        if(mouse.button === Qt.LeftButton) {
                            pageStack.push(Qt.resolvedUrl("EditPage.qml"), {documentUrl: fileUrl, cursorPos: cursorPosition})
                        }
                    }
                }
            }
        }
    }
}
