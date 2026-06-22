import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property double batteryPercent
    required property bool isCharging
    required property string networkStatus

    readonly property string displayNetworkStatus: networkStatus === "Disconnected" ? "Offline" : networkStatus
    readonly property color batteryFillColor: batteryPercent < 20 ? Theme.colors.danger : (isCharging ? Theme.colors.accent : Theme.colors.success)

    implicitHeight: 24
    implicitWidth: row.implicitWidth

    RowLayout {
        id: row
        anchors.centerIn: parent
        spacing: Theme.spacing.sm

        Rectangle {
            id: networkPill
            implicitHeight: 24
            implicitWidth: Math.max(96, networkLabel.implicitWidth + 26)
            radius: 999
            color: Theme.colors.surfaceRaised
            border.color: Theme.colors.border
            border.width: 1
            opacity: 0.92
            Layout.alignment: Qt.AlignVCenter

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacing.xs
                spacing: Theme.spacing.xs

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: networkStatus === "Disconnected" ? Theme.colors.textSecondary : Theme.colors.accent
                    opacity: 0.85
                    Layout.alignment: Qt.AlignVCenter
                }

                Text {
                    id: networkLabel
                    text: displayNetworkStatus
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    color: Theme.colors.textSecondary
                    elide: Text.ElideRight
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }

        Rectangle {
            id: batteryPill
            implicitHeight: 24
            implicitWidth: Math.max(108, batteryLabel.implicitWidth + 36)
            radius: 999
            color: Theme.colors.surfaceRaised
            border.color: Theme.colors.border
            border.width: 1
            opacity: 0.92
            Layout.alignment: Qt.AlignVCenter

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacing.xs
                spacing: Theme.spacing.xs

                Item {
                    width: 16
                    height: 10
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        anchors.fill: parent
                        radius: 3
                        border.width: 1
                        border.color: batteryFillColor
                        color: "transparent"
                    }

                    Rectangle {
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                            margins: 1
                        }
                        width: Math.max(2, (parent.width - 2) * batteryPercent / 100)
                        radius: 2
                        color: batteryFillColor
                    }

                    Rectangle {
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            rightMargin: -2
                        }
                        width: 2
                        height: 5
                        radius: 1
                        color: batteryFillColor
                    }
                }

                Text {
                    id: batteryLabel
                    text: Math.round(batteryPercent) + "%"
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    font.weight: Font.DemiBold
                    color: Theme.colors.textSecondary
                    Layout.alignment: Qt.AlignVCenter
                }

                Text {
                    text: isCharging ? "Charging" : (batteryPercent < 20 ? "Low" : "Battery")
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    color: batteryFillColor
                    opacity: 0.9
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }
    }
}
