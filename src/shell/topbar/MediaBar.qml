import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import Fluent

Rectangle {
    id: mediaRoot

    // Data properties bound to our unified C++ backend
    // This dynamically tracks if any media player (Spotify, browser, etc.) is active
    property bool hasMedia: typeof SystemState !== "undefined" ? SystemState.isMediaActive : false
    property bool isPlaying: typeof SystemState !== "undefined" ? SystemState.isMediaPlaying : false
    property string title: typeof SystemState !== "undefined" ? SystemState.mediaTitle : ""
    property real progress: typeof SystemState !== "undefined" ? SystemState.mediaProgress : 0.0

    // Component state helper
    readonly property bool isHovered: hoverHandler.hovered

    // Smoothly collapse the entire widget's dimensions to 0 when no media is present
    width: hasMedia ? 240 : 0
    implicitWidth: width
    implicitHeight: hasMedia ? 28 : 0 // Collapses height to prevent taking vertical layout space
    opacity: hasMedia ? 1.0 : 0.0
    visible: hasMedia && (opacity > 0) // Ensures it's fully culled from the scene graph when inactive
    clip: true
    radius: Theme.radius.sm
    color: isHovered ? Theme.colors.hoverOverlay : Theme.colors.surfaceRaised
    border.color: Theme.colors.borderSubtle
    border.width: hasMedia ? 1 : 0

    // Fluid transitions for visual state adjustments
    Behavior on implicitWidth {
        NumberAnimation {
            duration: Theme.timing.normal
            easing.type: Easing.OutCubic
        }
    }
    Behavior on implicitHeight {
        NumberAnimation {
            duration: Theme.timing.normal
            easing.type: Easing.OutCubic
        }
    }
    Behavior on opacity {
        NumberAnimation {
            duration: Theme.timing.normal
        }
    }
    Behavior on color {
        ColorAnimation {
            duration: Theme.timing.fast
        }
    }

    HoverHandler {
        id: hoverHandler
    }

    // =========================================================
    // THE LAYOUT CONTAINER
    // =========================================================
    RowLayout {
        id: contentLayout

        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.xs
        anchors.rightMargin: Theme.spacing.xs

        spacing: Theme.spacing.xs

        Row {
            id: controlsRow

            spacing: Theme.spacing.xs
            Layout.alignment: Qt.AlignVCenter

            // prev
            IconButton {
                id: prevBtn
                name: "skipprevious"
                onClicked: SystemState.mediaPrevious()
            }

            // play pause
            IconButton {
                id: pause
                name: isPlaying ? "pause" : "play"
                onClicked: SystemState.toggleMediaPlayPause()

            }

            // next
            IconButton {
                id: next
                name: "skipnext"
                onClicked: SystemState.mediaNext()

            }
        }

        Rectangle {
            width: 1
            height: 12
            color: Theme.colors.border
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            id: titleContainer

            Layout.fillWidth: true
            Layout.minimumWidth: 20
            Layout.alignment: Qt.AlignVCenter

            implicitHeight: titleText.implicitHeight

            clip: true

            Text {
                id: titleText

                text: mediaRoot.title

                anchors.verticalCenter: parent.verticalCenter

                color: Theme.colors.text
                font.pixelSize: Theme.fontSize.sm
                font.weight: Font.DemiBold

                width: implicitWidth
                // onTextChanged: marqueeAnim.restart()
            }
            HoverHandler {
                onHoveredChanged: {
                    marqueeAnim.restart()
                }
            }
        }
    }
    // =========================================================
    // 3. MICROSCOPIC PROGRESS UNDERLAY (At the very bottom)
    // =========================================================
    Rectangle {
        id: progressBarTrack
        height: 1.5
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: Theme.colors.borderSubtle
        radius: Theme.radius.sm
        visible: mediaRoot.hasMedia

        // The actual progress indicator
        Rectangle {
            id: progressBarFill
            height: parent.height
            anchors.left: parent.left
            width: parent.width * Math.max(0.0, Math.min(1.0, mediaRoot.progress))
            color: Theme.colors.accent
            radius: parent.radius

            Behavior on width {
                NumberAnimation {
                    duration: Theme.timing.normal
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}

// import QtQuick
// import QtQuick.Layouts
// import QtQuick.Shapes
// import Fluent

// Rectangle {
//     id: mediaRoot

//     property bool hasMedia: typeof SystemState !== "undefined" ? SystemState.isMediaActive : false
//     property bool isPlaying: typeof SystemState !== "undefined" ? SystemState.isMediaPlaying : false
//     property string title: typeof SystemState !== "undefined" ? SystemState.mediaTitle : ""
//     property real progress: typeof SystemState !== "undefined" ? SystemState.mediaProgress : 0.0

//     readonly property bool isHovered: hoverHandler.hovered

//     implicitWidth: hasMedia ? 220 : 0
//     implicitHeight: hasMedia ? 28 : 0
//     opacity: hasMedia ? 1.0 : 0.0
//     visible: hasMedia
//     clip: true

//     radius: height / 2
//     color: isHovered ? Theme.colors.hoverOverlay : Theme.colors.surfaceSoft
//     border.color: Theme.colors.borderSubtle
//     border.width: hasMedia ? 1 : 0

//     Behavior on opacity { NumberAnimation { duration: Theme.timing.normal } }
//     HoverHandler { id: hoverHandler }

//     RowLayout {
//         anchors.fill: parent
//         anchors.leftMargin: Theme.spacing.sm
//         anchors.rightMargin: Theme.spacing.sm
//         spacing: Theme.spacing.sm

//         // 1. Controls Zone (Fixed Width to prevent layout shifts)
//         Item {
//             id: controlsContainer
//             width: 50 // Fixed width prevents shifting when buttons appear
//             height: 16
//             Layout.alignment: Qt.AlignVCenter

//             Row {
//                 anchors.centerIn: parent
//                 spacing: 4

//                 // Prev
//                 Shape {
//                     width: 12; height: 12; opacity: isHovered ? 1 : 0
//                     Behavior on opacity { NumberAnimation { duration: 200 } }
//                     ShapePath {
//                         fillColor: Theme.colors.text; strokeColor: "transparent"
//                         PathSvg { path: "M 8 0 L 2 5 L 8 10 Z" }
//                     }
//                     TapHandler { onTapped: if (typeof SystemState !== "undefined") SystemState.mediaPrevious() }
//                 }

//                 // Play/Pause
//                 Shape {
//                     width: 12; height: 12
//                     ShapePath {
//                         fillColor: Theme.colors.accent; strokeColor: "transparent"
//                         PathSvg { path: mediaRoot.isPlaying ? "M 0 0 L 3 0 L 3 10 Z M 6 0 L 9 0 L 9 10 Z" : "M 1 0 L 9 5 L 1 10 Z" }
//                     }
//                     TapHandler { onTapped: if (typeof SystemState !== "undefined") SystemState.toggleMediaPlayPause() }
//                 }

//                 // Next
//                 Shape {
//                     width: 12; height: 12; opacity: isHovered ? 1 : 0
//                     Behavior on opacity { NumberAnimation { duration: 200 } }
//                     ShapePath {
//                         fillColor: Theme.colors.text; strokeColor: "transparent"
//                         PathSvg { path: "M 0 0 L 6 5 L 0 10 Z" }
//                     }
//                     TapHandler { onTapped: if (typeof SystemState !== "undefined") SystemState.mediaNext() }
//                 }
//             }
//         }

//         // 2. Marquee Text Zone
//         Item {
//             id: textMask
//             Layout.fillWidth: true
//             Layout.alignment: Qt.AlignVCenter
//             height: 16
//             clip: true

//             Text {
//                 id: marqueeText
//                 text: mediaRoot.title
//                 color: Theme.colors.text
//                 font.pixelSize: Theme.fontSize.sm
//                 font.weight: Theme.fontWeight.bold
//                 anchors.verticalCenter: parent.verticalCenter
//                 x: 0

//                 onTextChanged: marqueeAnim.restart()

//                 MouseArea {
//                     id: marqueTrigger
//                     hoverEnabled: true
//                     onEntered: marqueeAnim.restart()
//                 }

//                 SequentialAnimation {
//                     id: marqueeAnim
//                     running: isHovered || (title !== "")
//                     loops: Animation.InfiniteonIsHoveredChanged
//                     PauseAnimation { duration: 1000 }
//                     PropertyAnimation {
//                         target: marqueeText; property: "x"
//                         from: 0; to: -marqueeText.implicitWidth + textMask.width
//                         duration: (marqueeText.implicitWidth - textMask.width) * 30
//                         easing.type: Easing.Linear
//                     }
//                     PauseAnimation { duration: 1000 }
//                 }
//             }
//         }
//     }

//     // Progress Bar
//     Rectangle {
//         height: 1.5; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right
//         color: Theme.colors.borderSubtle
//         Rectangle {
//             height: parent.height; color: Theme.colors.accent
//             width: parent.width * mediaRoot.progress
//             Behavior on width { NumberAnimation { duration: 500 } }
//         }
//     }
// }
