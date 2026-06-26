// // src/shell/qml/BatteryIndicator.qml
// import QtQuick
// import QtQuick.Layouts
// import QtQuick.Shapes // Required for the premium vector lightning bolt

// RowLayout {
//     id: batteryRoot
    
//     required property real level      // 0 to 100
//     property bool isCharging: false
//     property bool showText: true

//     spacing: Theme.spacing.sm
//     Layout.alignment: Qt.AlignVCenter

//     // 1. Text moved OUTSIDE the battery for premium legibility
//     Text {
//         text: Math.round(batteryRoot.level) + "%"
//         color: Theme.colors.text
//         font.pixelSize: Theme.fontSize.sm
//         font.bold: true
//         visible: batteryRoot.showText
//         Layout.alignment: Qt.AlignVCenter
//     }

//     // 2. Battery Graphic Container
//     Item {
//         width: 28
//         height: 14
//         Layout.alignment: Qt.AlignVCenter

//         // The Battery Outline
//         Rectangle {
//             id: batteryOutline
//             width: 25
//             height: 14
//             anchors.left: parent.left
//             anchors.verticalCenter: parent.verticalCenter
//             color: "transparent"
//             border.color: Theme.colors.textSecondary
//             border.width: 1
//             radius: Theme.radius.sm
//             opacity: 0.6 // Keep outline subtle
//         }

//         // The Battery Terminal (The right bump)
//         Rectangle {
//             width: 3
//             height: 6
//             anchors.left: batteryOutline.right
//             anchors.verticalCenter: parent.verticalCenter
//             color: Theme.colors.textSecondary
//             radius: 1
//             opacity: 0.6
//         }

//         // The Inner Fill Level
//         Rectangle {
//             id: fillLevel
//             anchors.left: batteryOutline.left
//             anchors.top: batteryOutline.top
//             anchors.bottom: batteryOutline.bottom
//             anchors.margins: 2 // Premium 1px visual gap from the border
            
//             // Width math: (Total outline width - 4px margins) * percentage
//             width: Math.max(0, (batteryOutline.width - 4) * (batteryRoot.level / 100))
//             radius: 2
            
//             // Dynamic Color Logic
//             color: {
//                 if (batteryRoot.isCharging) return Theme.colors.success;
//                 if (batteryRoot.level <= 20) return Theme.colors.danger;
//                 if (batteryRoot.level <= 40) return Theme.colors.warning;
//                 return Theme.colors.text;
//             }

//             // Silky smooth animation when battery drains or charges
//             Behavior on width {
//                 NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic }
//             }
//             Behavior on color {
//                 ColorAnimation { duration: Theme.timing.normal }
//             }
//         }

//         // The Charging Shock/Lightning Icon
//         Shape {
//             anchors.centerIn: batteryOutline
//             width: 8
//             height: 12
//             visible: batteryRoot.isCharging
            
//             // Soft drop shadow on the bolt so it's visible over the green fill
//             layer.enabled: true
            
//             ShapePath {
//                 fillColor: Theme.colors.background
//                 strokeColor: "transparent"
//                 // Scalable vector points for a perfect lightning bolt
//                 PathSvg { path: "M 4 0 L 0 6 L 3 6 L 2 12 L 8 5 L 4 5 Z" }
//             }

//             // Subtle breathing effect on the lightning bolt while charging
//             SequentialAnimation on scale {
//                 loops: Animation.Infinite
//                 running: batteryRoot.isCharging
//                 NumberAnimation { from: 1.0; to: 0.8; duration: 1000; easing.type: Easing.InOutSine }
//                 NumberAnimation { from: 0.8; to: 1.0; duration: 1000; easing.type: Easing.InOutSine }
//             }
//         }
//     }
// }

// src/shell/qml/BatteryIndicator.qml
import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes

RowLayout {
    id: batteryRoot
    
    required property real level      // 0 to 100
    property bool isCharging: false

    spacing: 0
    Layout.alignment: Qt.AlignVCenter

    // Battery Graphic Container
    Item {
        id: batteryContainer
        width: 44
        height: 20
        Layout.alignment: Qt.AlignVCenter

        // 1. The Battery Outline Body (Resized for internal text breathing room)
        Rectangle {
            id: batteryOutline
            width: 41
            height: 20
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.colors.surfaceSoft
            border.color: Theme.colors.border
            border.width: 1.2
            radius: 5
        }

        // 2. The Battery Terminal (Right Edge Bump)
        Rectangle {
            width: 3
            height: 8
            anchors.left: batteryOutline.right
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.colors.border
            radius: 1.5
        }

        // 3. The Inner Fill Level
        Rectangle {
            id: fillLevel
            anchors.left: batteryOutline.left
            anchors.top: batteryOutline.top
            anchors.bottom: batteryOutline.bottom
            anchors.margins: 2 // Clean 2px visual gap inside the shell
            
            // Calculate width safely against parent bounds
            width: Math.max(0, (batteryOutline.width - 4) * (batteryRoot.level / 100))
            radius: 3.5
            
            // Utilizing your direct semantic colors from Theme.qml
            color: {
                if (batteryRoot.isCharging) return Theme.colors.success;
                if (batteryRoot.level <= 20) return Theme.colors.danger;
                if (batteryRoot.level <= 40) return Theme.colors.warning;
                return Theme.colors.text; // Falls back to default bright text color
            }

            Behavior on width {
                NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic }
            }
            Behavior on color {
                ColorAnimation { duration: Theme.timing.fast }
            }
        }

        // 4. Centralized Layout (Shock Icon + Text Label matching horizontal alignments)
        Row {
            id: internalContent
            anchors.centerIn: batteryOutline
            spacing: 2
            
            // PREMIUM DETAIL: If the battery fill covers the text (> 55%), 
            // switch text to the dark background color. Otherwise, keep it bright.
            readonly property color contentColor: {
                if (batteryRoot.level > 55) {
                    // Extract RGB channels from the current Theme background color and apply 60% opacity
                    return Qt.rgba(Theme.colors.background.r,
                                   Theme.colors.background.g,
                                   Theme.colors.background.b,
                                   0.6);
                }
                // Extract RGB channels from the current Theme text color and apply 80% opacity
                return Qt.rgba(Theme.colors.text.r,
                               Theme.colors.text.g,
                               Theme.colors.text.b,
                               0.8);
            }

            // Percentage Text
            Text {
                text: Math.round(batteryRoot.level) + "%"
                color: Theme.colors.text
                font.pixelSize: Theme.fontSize.xs // Evaluates to 11px via your Theme system
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                renderType: Text.QtRendering 
                Behavior on color {
                    ColorAnimation { duration: Theme.timing.fast }
                }
            }

            // Crisp Charging Shock Vector Icon
            Shape {
                width: 7
                height: 11
                anchors.verticalCenter: parent.verticalCenter
                visible: batteryRoot.isCharging
                
                ShapePath {
                    fillColor: Theme.colors.background
                    strokeColor: "transparent"
                    PathSvg { path: "M 3.5 0 L 0 5.5 L 2.5 5.5 L 1.5 11 L 7 4.5 L 3.5 4.5 Z" }
                }

                // Subtle premium pulse animation while plugged in
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    running: batteryRoot.isCharging
                    NumberAnimation { from: 1.0; to: 0.4; duration: 900; easing.type: Easing.InOutSine }
                    NumberAnimation { from: 0.4; to: 1.0; duration: 900; easing.type: Easing.InOutSine }
                }
            }

        }
    }
}