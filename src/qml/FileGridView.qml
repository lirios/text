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
    property int cardSize: Units.gu(5)

    anchors.fill: parent
    contentHeight: fileGrid.height

    GridView {
        id: fileGrid

        width: ~~((parent.width - 2*margins + spacing) / (cardSize+spacing)) * (cardSize+spacing)
        anchors.horizontalCenter: parent.horizontalCenter
        height: childrenRect.height
        cellWidth: cardSize + spacing
        cellHeight: cardSize + spacing

        delegate: Item {
            width: fileGrid.cellWidth
            height: fileGrid.cellHeight

            Card {
                id: fileCard
                anchors.fill: parent
                anchors.margins: spacing / 2

                Label {
                    id: docName
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: Units.dp(16)

                    text: name
                    font.pixelSize: Units.dp(24)
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
