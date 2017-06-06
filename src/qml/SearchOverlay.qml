import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
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

    Row {
        id: overlayContent
        leftPadding: 8
        spacing: 0
        padding: 0

        TextField {
            id: searchField
            width: 256
            anchors.verticalCenter: parent.verticalCenter
            selectByMouse: true
            Keys.onDownPressed: activated(text, true)
            Keys.onUpPressed: activated(text, false)
            Keys.onReturnPressed: activated(text, event.modifiers ^ Qt.ShiftModifier)
            Keys.onEscapePressed: close()
        }

        Rectangle {
            width: 12
            height: 1
        }

        FluidControls.IconButton {
            iconName: "hardware/keyboard_arrow_down"
            iconColor: enabled ? Material.iconColor : Material.color(Material.Grey)
            enabled: searchField.text.length > 0
            onClicked: activated(searchField.text, true)
        }

        FluidControls.IconButton {
            iconName: "hardware/keyboard_arrow_up"
            iconColor: enabled ? Material.iconColor : Material.color(Material.Grey)
            enabled: searchField.text.length > 0
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
