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

ApplicationWindow {
    id: app

    visible: true
    title: qsTr("Liri Text")

    minimumWidth: dp(384)
    minimumHeight: dp(256)

    initialPage: RecentFilesPage { }

    theme {
        primaryColor: "purple"
        accentColor: "deepOrange"
    }

    Component.onCompleted: {
        console.log("app completed")
        if(givenPath) {
            var cp = 0
            if(history.getFileInfo(givenPath))
                cp = history.getFileInfo(givenPath).cursorPosition
            pageStack.push(Qt.resolvedUrl("EditPage.qml"), { documentUrl: givenPath, cursorPos: cp })
        }
        if(newDoc) {
            pageStack.push(Qt.resolvedUrl("EditPage.qml"), { anonymous: true })
        }
    }
}
