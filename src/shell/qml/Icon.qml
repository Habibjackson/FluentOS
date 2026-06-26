import QtQuick

Image {
    required property string iconName

    source: "/assets/icons/" + iconName + ".svg"

    width: 16
    height: 16

    fillMode: Image.PreserveAspectFit
}
