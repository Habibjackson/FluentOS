import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string activeAppTitle: "Fluent Shell"
    property string currentTime: ""
    property string currentDate: ""
    property double batteryPercent: 100.0
    property bool isCharging: false
    property string networkStatus: "Disconnected"
    property bool isNetworkConnected: false

    implicitWidth: Constants.notificationWidth
    implicitHeight: 284

    Rectangle {
        id: shadow
        anchors.fill: parent
        x: 5
        y: 6
        radius: 22
        color: Theme.colors.overlay
        opacity: 0.6
    }

    Rectangle {
        id: panel
        anchors.fill: parent
        radius: 22
        color: Theme.colors.surfaceRaised
        border.color: Theme.colors.border
        border.width: 1
        opacity: 0.98
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacing.lg
        spacing: Theme.spacing.md

        RowLayout {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 1

                Text {
                    text: "Notification Center"
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.lg
                    font.weight: Font.DemiBold
                    color: Theme.colors.text
                }

                Text {
                    text: "Live shell summary"
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    color: Theme.colors.textSecondary
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: currentDate + "  " + currentTime
                font.family: Theme.fonts.defaultFont.family
                font.pixelSize: Theme.fontSize.xs
                color: Theme.colors.textSecondary
                horizontalAlignment: Text.AlignRight
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
            radius: 18
            color: Theme.colors.surfaceSoft
            border.color: Theme.colors.border
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacing.md
                spacing: 2

                Text {
                    text: "Focused window"
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    color: Theme.colors.textSecondary
                }

                Text {
                    text: activeAppTitle.length ? activeAppTitle : "Fluent Shell"
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.lg
                    font.weight: Font.DemiBold
                    color: Theme.colors.text
                    elide: Text.ElideRight
                }

                Text {
                    text: "This is the title shown for the active app."
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    color: Theme.colors.textSecondary
                    elide: Text.ElideRight
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacing.sm

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                radius: 16
                color: Theme.colors.surfaceSoft
                border.color: Theme.colors.border
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacing.md
                    spacing: 2

                    Text {
                        text: "Battery"
                        font.family: Theme.fonts.defaultFont.family
                        font.pixelSize: Theme.fontSize.xs
                        color: Theme.colors.textSecondary
                    }

                    Text {
                        text: Math.round(batteryPercent) + "%"
                        font.family: Theme.fonts.defaultFont.family
                        font.pixelSize: Theme.fontSize.xl
                        font.weight: Font.DemiBold
                        color: batteryPercent < 20 ? Theme.colors.danger : Theme.colors.text
                    }

                    Text {
                        text: isCharging ? "Charging now" : "Power status"
                        font.family: Theme.fonts.defaultFont.family
                        font.pixelSize: Theme.fontSize.xs
                        color: isCharging ? Theme.colors.accent : Theme.colors.textSecondary
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 72
                radius: 16
                color: Theme.colors.surfaceSoft
                border.color: Theme.colors.border
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacing.md
                    spacing: 2

                    Text {
                        text: "Network"
                        font.family: Theme.fonts.defaultFont.family
                        font.pixelSize: Theme.fontSize.xs
                        color: Theme.colors.textSecondary
                    }

                    Text {
                        text: networkStatus === "Disconnected" ? "Offline" : networkStatus
                        font.family: Theme.fonts.defaultFont.family
                        font.pixelSize: Theme.fontSize.xl
                        font.weight: Font.DemiBold
                        color: isNetworkConnected ? Theme.colors.text : Theme.colors.textSecondary
                        elide: Text.ElideRight
                    }

                    Text {
                        text: isNetworkConnected ? "Connected" : "No connection"
                        font.family: Theme.fonts.defaultFont.family
                        font.pixelSize: Theme.fontSize.xs
                        color: isNetworkConnected ? Theme.colors.success : Theme.colors.textSecondary
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            radius: 16
            color: Theme.colors.surfaceSoft
            border.color: Theme.colors.border
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.spacing.md

                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: Theme.colors.accent
                    opacity: 0.9
                }

                Text {
                    text: "No new notifications"
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.sm
                    color: Theme.colors.text
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: "Calendar, mail, and app alerts will appear here."
                    font.family: Theme.fonts.defaultFont.family
                    font.pixelSize: Theme.fontSize.xs
                    color: Theme.colors.textSecondary
                }
            }
        }
    }
}
