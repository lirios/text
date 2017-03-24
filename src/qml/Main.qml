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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import Fluid.Controls 1.0 as FluidControls
import io.liri.text 1.0

FluidControls.FluidWindow {
    id: app

    visible: true
    title: qsTr("Liri Text")

    minimumWidth: 384
    minimumHeight: 256

    width: 960
    height: 640

    initialPage: RecentFilesPage { }

    Material.primary: Material.Purple
    Material.accent: Material.DeepOrange

    Component.onCompleted: {
        console.log("app completed")
        if(givenPath) {
            var cp = 0
            if(History.getFileInfo(givenPath))
                cp = History.getFileInfo(givenPath).cursorPosition
            pageStack.push(Qt.resolvedUrl("EditPage.qml"), { documentUrl: givenPath, cursorPos: cp })
        }
        if(newDoc) {
            pageStack.push(Qt.resolvedUrl("EditPage.qml"), { anonymous: true })
        }
    }
}
