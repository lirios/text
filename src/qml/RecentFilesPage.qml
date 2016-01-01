import QtQuick 2.4
import Material 0.1
import QtQuick.Controls 1.4 as Controls

Page {
    id: page

    actionBar.title: qsTr("Recent Files")
    GridView {
        id: recentFilesView

        anchors.fill: parent
        model: 1
        delegate: Item {
            width: recentFilesView.width / 2
            height: width

            Card {
                id: newFile
                anchors.fill: parent
                anchors.margins: Units.dp(14)
                Icon {
                    id: plusIcon
                    anchors.centerIn: parent
                    size: parent.width * 2/3
                    name: "content/add"
                }

                Ink {
                    id: animation
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    onClicked: {
                        pageStack.push(Qt.resolvedUrl("EditPage.qml"))
                    }
                }
            }
        }
    }
}
