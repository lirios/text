import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import Fluid.Controls 1.0 as FluidControls

FluidControls.Card {
    id: overlay
    signal activated(string query)
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
    width: 256
    height: searchField.height
    Material.elevation: 2

    TextField {
        id: searchField
        width: parent.width - 2*8
        anchors.centerIn: parent
        Keys.onReturnPressed: activated(text)
        Keys.onEscapePressed: close()
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
