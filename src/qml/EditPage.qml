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
import Fluid.Controls 1.0 as FluidControls
import QtQuick.Controls 2.1
import QtQuick.Dialogs 1.2 as Dialogs
import io.liri.text 1.0

FluidControls.Page {
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

    function touchFileOnCursorPosition() {
        history.touchFile(document.documentTitle, documentUrl, mainArea.cursorPosition,
                          document.textFragment(mainArea.cursorPosition, 7))
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

    title: anonymous ? qsTr("New Document") : document.documentTitle
    appBar.maxActionCount: 1

    actions: [
        FluidControls.Action {
            id: saveAction
            iconName: "content/save"
            tooltip: qsTr("Save")
            shortcut: "Ctrl+S"
            onTriggered: save()
        },

        FluidControls.Action {
            id: saveAsAction
            iconName: "content/save"
            text: qsTr("Save As")
            shortcut: "Ctrl+Shift+S"
            onTriggered: saveAs()
        },

        FluidControls.Action {
            id: closeAction
            iconName: "navigation/close"
            text: qsTr("Close")
            shortcut: "Ctrl+W"
            onTriggered: page.pop()
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
            save()
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
            exitDialog.close()
            exitDialog.accepted.connect(onAccepted)
            exitDialog.rejected.connect(onRejected)
            exitDialog.closed.connect(onClosed)
            exitDialog.open()
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
                    save()
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
                    exitDialog.close()
                    exitDialog.accepted.connect(onAccepted)
                    exitDialog.rejected.connect(onRejected)
                    exitDialog.closed.connect(onClosed)
                    exitDialog.open()
                } else {
                    touchFileOnCursorPosition()
                }
            }
        }
    }

    FluidControls.Dialog {
        id: exitDialog
        title: qsTr("Save changes before closing?")
        text: qsTr("You have unsaved changes. Do you want to save them before closing the file?")

        positiveButtonText: qsTr("Yes")
        negativeButtonText: qsTr("No")
        //cancelButtonText: qsTr("Cancel")
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

    FluidControls.Dialog {
        id: askForReloadDialog

        title: qsTr("Reload file content?")
        text: qsTr("The file was changed from outside. Do you wish to reload its content?")

        positiveButtonText: qsTr("Reload")
        negativeButtonText: qsTr("Ignore")

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

    Flickable {
        id: flickable
        anchors.fill: parent

        TextArea.flickable: TextArea {
            id: mainArea
            focus: true
            persistentSelection: true
            selectByMouse: true
            textMargin: 8
            font: defaultFont
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: document.text

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.IBeamCursor
                acceptedButtons: Qt.RightButton
                onClicked: contextMenu.openAt(mouse.x, mouse.y)
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }

    Menu {
        id: contextMenu

        function openAt(x, y) {
            contextMenu.x = x
            contextMenu.y = y
            contextMenu.open()
        }

        MenuItem {
            text: qsTr("Copy")
            enabled: mainArea.selectedText
            onTriggered: mainArea.copy()
        }
        MenuItem {
            text: qsTr("Cut")
            enabled: mainArea.selectedText
            onTriggered: mainArea.cut()
        }
        MenuItem {
            text: qsTr("Paste")
            enabled: mainArea.canPaste
            onTriggered: mainArea.paste()
        }

        MenuSeparator { }

        MenuItem {
            text: qsTr("Select All")
            onTriggered: mainArea.selectAll()
        }

        MenuSeparator { }

        MenuItem {
            text: qsTr("Undo")
            enabled: mainArea.canUndo
            onTriggered: mainArea.undo()
        }
        MenuItem {
            text: qsTr("Redo")
            enabled: mainArea.canRedo
            onTriggered: mainArea.redo()
        }
    }

    DocumentHandler {
        id: document
        target: mainArea

        onFileChangedOnDisk: {
            console.log("file changed on disk")
            askForReloadDialog.open()
        }

        onError: {
            //app.showError(qsTr("File operation error"), description)
            errDiag.text = description
            errDiag.open()
        }
    }

    FluidControls.Dialog {
        id: errDiag
        title: qsTr("File operation error")

        positiveButtonText: qsTr("Ok")
        negativeButton.visible: false
    }
}
