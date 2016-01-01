import QtQuick 2.4
import Material 0.1

ApplicationWindow {
    id: app

    visible: true
    title: qsTr("Liri Text")

    initialPage: RecentFilesPage { }

    theme {
        primaryColor: "deepPurple"
    }
}
