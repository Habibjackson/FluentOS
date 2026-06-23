import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string title: "Fluent Shell"

    implicitHeight: 24
    implicitWidth: titleRow.implicitWidth + Theme.spacing.sm

    RowLayout {
        id: titleRow
        anchors.left: parent.left
        spacing: Theme.spacing.xs

        Rectangle {
            width: 8
            height: 8
            radius: 4
            color: Theme.colors.accent
            opacity: 0.9
            Layout.alignment: Qt.AlignVCenter
        }

        Text {
            text: root.title.length ? root.title : "Fluent Shell"
            font.family: Theme.fonts.defaultFont.family
            font.pixelSize: Theme.fontSize.sm + 1
            font.weight: Font.DemiBold
            color: Theme.colors.text
            elide: Text.ElideLeft
            Layout.alignment: Qt.AlignVCenter
        }
    }
}
