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
import Material.ListItems 0.1 as ListItems
import QtQuick.Controls 1.4 as Controls
import QtQuick.Dialogs 1.2 as Dialogs
import me.liriproject.text 1.0

Page {
    id: page
    property url documentUrl
    property bool anonymous: false

    function save() {
        if(anonymous)
            saveAs()
        else
            document.saveAs(documentUrl)
    }

    function saveAs() {
        saveAsDialog.open()
    }

    actionBar.title: anonymous ? qsTr("New Document") : document.documentTitle
    actionBar.maxActionCount: 2
    actions: [
        Action {
            id: saveAction
            iconName: "content/save"
            name: qsTr("Save")
            onTriggered: save()
        },

        Action {
            id: saveAsAction
            name: qsTr("Save As")
            onTriggered: saveAs()
        },

        Action {
            id: closeAction
            name: qsTr("Close")
            onTriggered: page.backAction.trigger()
        }
    ]

    Component.onCompleted: {
        if(!anonymous)
            history.touchFile(document.documentTitle, documentUrl, [mainArea.getText(0, 500)])
    }

    Dialogs.FileDialog {
        id: saveAsDialog
        title: qsTr("Choose a location to save")
        selectExisting: false

        onAccepted: {
            document.saveAs(saveAsDialog.fileUrl)
            documentUrl = saveAsDialog.fileUrl
            anonymous = false
            history.touchFile(document.documentTitle, documentUrl, [mainArea.getText(0, 500)])
        }
    }

    Shortcut {
        sequence: "Ctrl+S"
        onActivated: saveAction.trigger()
    }

    Shortcut {
        sequence: "Ctrl+Shift+S"
        onActivated: saveAsAction.trigger()
    }

    Shortcut {
        sequence: "Ctrl+W"
        onActivated: closeAction.trigger()
    }

    Controls.TextArea {
        id: mainArea
        anchors.fill: parent
        textMargin: Units.dp(16)
        focus: true
        font.family: "Roboto"
        font.pixelSize: Units.dp(18)
        font.weight: Font.Normal
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        text: document.text
    }

    Dialog {
        id: askForReloadDialog

        title: qsTr("Reload file content?")
        text: qsTr("The file was changed from outside. Do you wish to reload its content?")

        positiveButtonText: qsTr("RELOAD")
        negativeButtonText: qsTr("IGNORE")

        onAccepted: document.reloadText()
    }

    DocumentHandler {
        id: document
        target: mainArea
        fileUrl: documentUrl

        onFileChangedOnDisk: {
            console.log("file changed on disk")
            askForReloadDialog.show()
        }
    }
}
