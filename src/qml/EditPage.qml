import QtQuick 2.4
import Material 0.1
import QtQuick.Controls 1.4 as Controls
import text.processing 1.0

Page {
    id: page
    property string filename

    actionBar.title: document.documentTitle
    actions: [
        Action {
            iconName: "content/save"
            name: qsTr("Save")
            onTriggered: {
                document.saveAs(filename)
            }
        }
    ]

    Controls.TextArea {
        id: mainArea
        anchors.fill: parent
        focus: true
        text: document.text
    }

    DocumentManager {
        id: document
        target: mainArea
        Component.onCompleted: {
            console.log(filename);
            document.fileUrl = filename
        }
    }
}
