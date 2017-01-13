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
import Material 0.3
import Material.ListItems 0.1 as ListItems
import QtQuick.Controls 1.4 as Controls
import QtQuick.Dialogs 1.2 as Dialogs
import io.liri.text 1.0

Page {
    id: page

    property url documentUrl
    property bool anonymous: false
    property alias document: document
    property int cursorPos: 0

    signal ioSuccess
    signal ioFailure

    function save() {
        if(anonymous)
            saveAs()
        else {
            if(document.saveAs(documentUrl)) {
                ioSuccess()
                touchFileOnCursorPosition()
            } else {
                ioFailure()
            }
        }
    }

    function saveAs() {
        saveAsDialog.open()
    }

    function getTextAroundLine(lineNum, height) {
        var lines = mainArea.text.split('\n')
        var start = lineNum - Math.floor(height/2)
        if(start < 0)
            start = 0
        var end = start + height - 1
        if(end >= lines.length) {
            start -= end - lines.length + 1
            if(start < 0)
                start = 0
            end = lines.length - 1
        }
        var res = []
        for(var i = start; i <= end; i++) {
            res.push(lines[i])
        }
        return res
    }

    function touchFileOnCursorPosition() {
        history.touchFile(document.documentTitle, documentUrl, mainArea.cursorPosition,
                          getTextAroundLine(mainArea.text.slice(0, mainArea.cursorPosition).split('\n').length - 1, 7))
    }

    Component.onCompleted: {
        console.log("edit page completed")

        if(document.setFileUrl(documentUrl)) {
            mainArea.cursorPosition = cursorPos
            if(!anonymous) {
                touchFileOnCursorPosition()
            }
        }
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

    onGoBack: {
        function forcedClose() {
            page.forcePop()
        }

        function onAccepted() {
            ioSuccess.connect(forcedClose)
            ioFailure.connect(function disc() {
                ioSuccess.disconnect(forcedClose)
                ioFailure.disconnect(disc)
            })
            saveAction.trigger()
        }

        function onRejected() {
            forcedClose()
        }

        function onClosed() {
            disconnectAll()
        }

        function disconnectAll() {
            exitDialog.accepted.disconnect(onAccepted)
            exitDialog.rejected.disconnect(onRejected)
            exitDialog.closed.disconnect(onClosed)
        }

        if(page.document.modified) {
            event.accepted = true
            if(exitDialog.showing)
                exitDialog.close()
            exitDialog.accepted.connect(onAccepted)
            exitDialog.rejected.connect(onRejected)
            exitDialog.closed.connect(onClosed)
            exitDialog.show()
        } else {
            touchFileOnCursorPosition()
        }
    }

    Connections {
        property bool forceClose: false

        target: app
        onClosing: {
            if(!forceClose) {
                function forcedClose() {
                    forceClose = true
                    app.close()
                }

                function onAccepted() {
                    ioSuccess.connect(forcedClose)
                    ioFailure.connect(function disc() {
                        ioSuccess.disconnect(forcedClose)
                        ioFailure.disconnect(disc)
                    })
                    saveAction.trigger()
                }

                function onRejected() {
                    forcedClose()
                }

                function onClosed() {
                    disconnectAll()
                }

                function disconnectAll() {
                    exitDialog.accepted.disconnect(onAccepted)
                    exitDialog.rejected.disconnect(onRejected)
                    exitDialog.closed.disconnect(onClosed)
                }

                if(page.document.modified) {
                    close.accepted = false
                    if(exitDialog.showing)
                        exitDialog.close()
                    exitDialog.accepted.connect(onAccepted)
                    exitDialog.rejected.connect(onRejected)
                    exitDialog.closed.connect(onClosed)
                    exitDialog.show()
                } else {
                    touchFileOnCursorPosition()
                }
            }
        }
    }

    ThreeButtonDialog {
        id: exitDialog
        title: qsTr("Save changes before closing?")
        text: qsTr("You have unsaved changes. Do you want to save them before closing the file?")

        positiveButtonText: qsTr("YES")
        negativeButtonText: qsTr("NO")
        cancelButtonText: qsTr("CANCEL")
    }

    Dialogs.FileDialog {
        id: saveAsDialog

        title: qsTr("Choose a location to save")
        selectExisting: false

        onAccepted: {
            if(document.saveAs(saveAsDialog.fileUrl)) {
                ioSuccess()
                documentUrl = saveAsDialog.fileUrl
                anonymous = false
                touchFileOnCursorPosition()
            } else {
                ioFailure()
            }
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

    Dialog {
        id: askForReloadDialog

        title: qsTr("Reload file content?")
        text: qsTr("The file was changed from outside. Do you wish to reload its content?")

        positiveButtonText: qsTr("RELOAD")
        negativeButtonText: qsTr("IGNORE")

        onAccepted: {
            var cp = mainArea.cursorPosition
            if(document.reloadText())
                ioSuccess()
            else
                ioFailure()
            mainArea.forceActiveFocus()
            mainArea.cursorPosition = cp
        }
    }

    Controls.TextArea {
        id: mainArea

        anchors.fill: parent
        textMargin: dp(8)
        focus: true
        font: defaultFont
//        font.family: "Roboto"
//        font.pixelSize: dp(16)
//        font.weight: Font.Normal
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        text: document.text
    }

    DocumentHandler {
        id: document
        target: mainArea

        onFileChangedOnDisk: {
            console.log("file changed on disk")
            askForReloadDialog.show()
        }

        onError: {
            //app.showError(qsTr("File operation error"), description)
            errDiag.text = description
            errDiag.show()
        }
    }

    Dialog {
        id: errDiag
        title: qsTr("File operation error")

        positiveButtonText: qsTr("OK")
        negativeButton.visible: false
    }
}
