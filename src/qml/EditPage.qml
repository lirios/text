import QtQuick 2.4
import Material 0.1
import QtQuick.Controls 1.4 as Controls
import text.processing 1.0

Page {
    id: page
    property string filename

    actionBar.title: filename
    actions: [
        Action {
            iconName: "content/save"
            name: qsTr("Save")
            onTriggered: {
                fp.fileContent = mainArea.text
            }
        }
    ]

    FileProcessor {
        id: fp
        fileName: filename
        //Component.onCompleted: {
        //    fp.open(filename)
        //    mainArea.text = fp.readAll()
        //}
    }

    Controls.TextArea {
        id: mainArea
        anchors.fill: parent
        focus: true
        text: fp.fileContent
    }
}
