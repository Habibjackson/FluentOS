import QtQuick
import QtQuick.Layouts

Item {
    id: clock

    required property string currentTime
    required property string currentDate

    property color backgroundColor: Theme.colors.surfaceRaised
    property color borderColor: Theme.colors.border
    property color timeColor: Theme.colors.text
    property color dateColor: Theme.colors.textSecondary
    property int borderWidth: 1

    implicitWidth: contentRow.implicitWidth + Theme.spacing.lg
    implicitHeight: 24

    Rectangle {
        id: clockCard
        anchors.fill: parent
        radius: 999
        color: backgroundColor
        border.color: borderColor
        border.width: borderWidth
        opacity: 0.92
    }

    RowLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: Theme.spacing.xs

        Text {
            text: clock.currentTime
            font.pixelSize: Theme.fontSize.sm
            font.weight: Font.DemiBold
            color: timeColor
            Layout.alignment: Qt.AlignVCenter
        }

        Text {
            text: "•"
            font.family: Theme.fonts.defaultFont.family
            font.pixelSize: Theme.fontSize.xs
            color: Theme.colors.textSecondary
            opacity: 0.7
            Layout.alignment: Qt.AlignVCenter
        }

        Text {
            text: clock.currentDate
            font.family: Theme.fonts.defaultFont.family
            font.pixelSize: Theme.fontSize.xs
            color: dateColor
            Layout.alignment: Qt.AlignVCenter
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            // Future: open calendar popup
        }
    }
}
