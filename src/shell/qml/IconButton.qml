// IconButton.qml

import QtQuick

Rectangle {
    id: root

    signal clicked()

    property string name

    width: 24
    height: 24
    radius: 4

    color: hover.hovered
           ? Theme.colors.hoverOverlay
           : "transparent"

    Image {
        anchors.centerIn: parent

        source: Qt.resolvedUrl("../../assets/icons/" + name + ".svg")

        width: 16
        height: 16

        Item {

        }

        fillMode: Image.PreserveAspectFit
    }

    HoverHandler {
        id: hover
    }

    TapHandler {
        onTapped: root.clicked()
    }
}