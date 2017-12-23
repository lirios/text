import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.1
import Fluid.Controls 1.0 as FluidControls

FluidControls.Card {
    id: overlay
    signal activated(string query, bool forward)
    signal closed

    function open() {
        state = "exposed"
        searchField.forceActiveFocus()
        searchField.selectAll()
    }

    function close() {
        state = "hidden"
        searchField.focus = false
        closed()
    }

    state: "hidden"
    width: overlayContent.width
    height: overlayContent.height
    Material.elevation: 2

    RowLayout {
        id: overlayContent
        width: 344
        height: searchField.height + 2*4

        TextField {
            id: searchField

            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.fillWidth: true

            selectByMouse: true
            Keys.onDownPressed: activated(text, true)
            Keys.onUpPressed: activated(text, false)
            Keys.onReturnPressed: activated(text, event.modifiers ^ Qt.ShiftModifier)
            Keys.onEscapePressed: close()
        }

        ToolButton {
            icon.name: "hardware/keyboard_arrow_down"
            icon.color: enabled ? Material.iconColor : Material.color(Material.Grey)
            enabled: searchField.text.length > 0

            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 0
            Layout.rightMargin: 0
            Layout.maximumWidth: 24 + 2*4

            onClicked: activated(searchField.text, true)
        }

        ToolButton {
            icon.name: "hardware/keyboard_arrow_up"
            icon.color: enabled ? Material.iconColor : Material.color(Material.Grey)
            enabled: searchField.text.length > 0

            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 0
            Layout.rightMargin: 8
            Layout.maximumWidth: 24 + 2*4

            onClicked: activated(searchField.text, false)
        }
    }

    Timer {
        id: hideTimer
        interval: 5000
        running: state === "exposed" && !overlay.focus
        onTriggered: overlay.close()
    }

    states: [
        State {
            name: "hidden"
            PropertyChanges {
                target: overlay
                y: -height
                visible: false
            }
        },
        State {
            name: "exposed"
            PropertyChanges {
                target: overlay
                y: 0
                visible: true
            }
        }
    ]

    transitions: [
        Transition {
            to: "exposed"
            SequentialAnimation {
                PropertyAction {
                    target: overlay
                    property: "visible"
                }
                NumberAnimation {
                    target: overlay
                    property: "y"
                    easing.type: Easing.OutQuad
                    duration: 200
                }
            }
        },

        Transition {
            to: "hidden"
            SequentialAnimation {
                NumberAnimation {
                    target: overlay
                    property: "y"
                    easing.type: Easing.InQuad
                    duration: 200
                }
                PropertyAction {
                    target: overlay
                    property: "visible"
                }
            }
        }
    ]
}
