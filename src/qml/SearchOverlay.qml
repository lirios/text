import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import Fluid.Controls 1.0 as FluidControls

FluidControls.Card {
    signal activated(string query)

    function open() {
        visible = true
        searchField.forceActiveFocus()
    }

    function close() {
        visible = false
    }

    visible: false

    width: 256
    height: searchField.height
    Material.elevation: 2

    TextField {
        id: searchField
        width: parent.width - 2*8
        anchors.centerIn: parent
        Keys.onReturnPressed: activated(text)
    }
}
