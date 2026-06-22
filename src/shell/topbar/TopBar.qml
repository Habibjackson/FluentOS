import QtQuick
import QtQuick.Layouts

Item {
    id: topBar
    height: Constants.topBarHeight

    property bool notificationCenterOpen: false

    readonly property string activeAppTitle: SystemState.activeAppTitle
    readonly property string currentTime: SystemState.currentTime
    readonly property string currentDate: SystemState.currentDate
    readonly property double batteryPercent: SystemState.batteryPercent
    readonly property bool isCharging: SystemState.isCharging
    readonly property string networkStatus: SystemState.networkStatus
    readonly property bool isNetworkConnected: SystemState.isNetworkConnected

    Rectangle {
        anchors.fill: parent
        color: Theme.colors.surface
        opacity: 0.92
        border.color: Theme.colors.border
        border.width: 1
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.colors.overlay
        opacity: 0.55
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.lg
        anchors.rightMargin: Theme.spacing.lg
        anchors.topMargin: Theme.spacing.sm
        anchors.bottomMargin: Theme.spacing.sm
        spacing: Theme.spacing.sm

        ApplicationTitle {
            Layout.preferredWidth: 280
            Layout.minimumWidth: 160
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            title: topBar.activeAppTitle
        }

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            spacing: Theme.spacing.sm
        
            Clock {
                Layout.alignment: Qt.AlignVCenter
                currentTime: topBar.currentTime
                currentDate: topBar.currentDate
            }

            Item {
                id: notificationButton
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24

                Rectangle {
                    anchors.fill: parent
                    radius: 12
                    color: topBar.notificationCenterOpen ? Theme.colors.accentLight : "transparent"
                    border.width: 1
                    border.color: topBar.notificationCenterOpen ? Theme.colors.accent : Theme.colors.border
                    opacity: topBar.notificationCenterOpen ? 0.95 : 0.8
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 1

                    Rectangle {
                        width: 10
                        height: 2
                        radius: 1
                        color: Theme.colors.text
                        opacity: 0.85
                    }

                    Rectangle {
                        width: 6
                        height: 2
                        radius: 1
                        color: Theme.colors.textSecondary
                        opacity: 0.75
                    }

                    Rectangle {
                        width: 8
                        height: 2
                        radius: 1
                        color: Theme.colors.text
                        opacity: 0.85
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: topBar.notificationCenterOpen = !topBar.notificationCenterOpen
                }
            }

            StatusArea {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                batteryPercent: topBar.batteryPercent
                isCharging: topBar.isCharging
                networkStatus: topBar.networkStatus
            }
        }
    }

    NotificationCenter {
        anchors.top: parent.bottom
        anchors.topMargin: Theme.spacing.sm
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacing.lg
        z: 2000
        visible: topBar.notificationCenterOpen
        activeAppTitle: topBar.activeAppTitle
        currentTime: topBar.currentTime
        currentDate: topBar.currentDate
        batteryPercent: topBar.batteryPercent
        isCharging: topBar.isCharging
        networkStatus: topBar.networkStatus
        isNetworkConnected: topBar.isNetworkConnected
    }
}
