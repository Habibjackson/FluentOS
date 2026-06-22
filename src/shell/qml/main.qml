import QtQuick
import QtQuick.Window

Window {
    width: 1920
    height: 1080
    visible: true
    color: Theme.colors.background
    title: SystemState.activeAppTitle.length ? SystemState.activeAppTitle : "Fluent Shell"

    TopBar {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        z: 1000
    }

    Rectangle {
        anchors {
            top: parent.top
            topMargin: Constants.topBarHeight
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        color: Theme.colors.background
    }
}
