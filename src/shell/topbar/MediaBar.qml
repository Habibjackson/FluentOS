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
    implicitWidth: hasMedia ? contentLayout.implicitWidth + (Theme.spacing.md * 2) : 0
    implicitHeight: hasMedia ? 28 : 0 // Collapses height to prevent taking vertical layout space
    opacity: hasMedia ? 1.0 : 0.0
    visible: hasMedia && (opacity > 0) // Ensures it's fully culled from the scene graph when inactive
    clip: true

    // Background Styling (Pill shape)
    radius: height / 2
    color: isHovered ? Theme.colors.hoverOverlay : Theme.colors.surfaceSoft
    border.color: Theme.colors.borderSubtle
    border.width: hasMedia ? 1 : 0

    // Fluid transitions for visual state adjustments
    Behavior on implicitWidth { NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic } }
    Behavior on implicitHeight { NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic } }
    Behavior on opacity { NumberAnimation { duration: Theme.timing.normal } }
    Behavior on color { ColorAnimation { duration: Theme.timing.fast } }

    HoverHandler { id: hoverHandler }

    // =========================================================
    // THE LAYOUT CONTAINER
    // =========================================================
    RowLayout {
        id: contentLayout
        anchors.centerIn: parent
        spacing: Theme.spacing.sm
        visible: mediaRoot.hasMedia // Prevent rendering children when hidden

        // 1. DYNAMIC CONTROLS ZONE (Expands on Hover)
        RowLayout {
            id: controlsRow
            spacing: Theme.spacing.xs
            Layout.alignment: Qt.AlignVCenter

            // A. Skip Previous Button (Only visible on hover)
            Item {
                id: prevButton
                width: mediaRoot.isHovered ? 16 : 0
                height: 16
                opacity: mediaRoot.isHovered ? 0.7 : 0.0
                visible: width > 0

                Behavior on width { NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic } }
                Behavior on opacity { NumberAnimation { duration: Theme.timing.fast } }

                Shape {
                    anchors.centerIn: parent
                    width: 8
                    height: 10
                    ShapePath {
                        fillColor: prevHover.hovered ? Theme.colors.accent : Theme.colors.text
                        strokeColor: "transparent"
                        PathSvg { path: "M 8 0 L 2 5 L 8 10 Z M 1 0 L 1 10 Z" }
                    }
                }
                HoverHandler { id: prevHover }
                TapHandler {
                    onTapped: {
                        if (typeof SystemState !== "undefined") SystemState.mediaPrevious();
                    }
                }
            }

            // B. Core Play/Pause Toggle Button (Always visible)
            Item {
                id: playPauseButton
                width: 18
                height: 18
                Layout.alignment: Qt.AlignVCenter

                // Play/Pause Vector Icon
                Shape {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    
                    ShapePath {
                        fillColor: playPauseHover.hovered ? Theme.colors.accent : Theme.colors.text
                        strokeColor: "transparent"
                        
                        // Dynamically morph the path depending on playback state
                        PathSvg {
                            path: mediaRoot.isPlaying 
                                  ? "M 0 0 L 3 0 L 3 10 L 0 10 Z M 6 0 L 9 0 L 9 10 L 6 10 Z" // Pause bars
                                  : "M 1 0 L 9 5 L 1 10 Z" // Play triangle
                        }
                    }
                }
                HoverHandler { id: playPauseHover }
                TapHandler {
                    onTapped: {
                        if (typeof SystemState !== "undefined") {
                            SystemState.toggleMediaPlayPause();
                        } else {
                            mediaRoot.isPlaying = !mediaRoot.isPlaying;
                        }
                    }
                }
            }

            // C. Skip Next Button (Only visible on hover)
            Item {
                id: nextButton
                width: mediaRoot.isHovered ? 16 : 0
                height: 16
                opacity: mediaRoot.isHovered ? 0.7 : 0.0
                visible: width > 0

                Behavior on width { NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic } }
                Behavior on opacity { NumberAnimation { duration: Theme.timing.fast } }

                Shape {
                    anchors.centerIn: parent
                    width: 8
                    height: 10
                    ShapePath {
                        fillColor: nextHover.hovered ? Theme.colors.accent : Theme.colors.text
                        strokeColor: "transparent"
                        PathSvg { path: "M 0 0 L 6 5 L 0 10 Z M 7 0 L 7 10 Z" }
                    }
                }
                HoverHandler { id: nextHover }
                TapHandler {
                    onTapped: {
                        if (typeof SystemState !== "undefined") SystemState.mediaNext();
                    }
                }
            }
        }

        // Separator bar between controls and text
        Rectangle {
            width: 1
            height: 12
            color: Theme.colors.border
            Layout.alignment: Qt.AlignVCenter
        }

        // 2. TYPOGRAPHY ZONE (Clean Track Title)
        Text {
            id: titleText
            text: mediaRoot.title
            color: Theme.colors.text
            font.pixelSize: Theme.fontSize.sm
            font.weight: Theme.fontWeight.bold
            renderType: Text.QtRendering
            elide: Text.ElideRight
            
            // Constrain layout width so it doesn't push adjacent components
            Layout.maximumWidth: mediaRoot.isHovered ? 140 : 180
            Layout.alignment: Qt.AlignVCenter

            Behavior on Layout.maximumWidth {
                NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic }
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
        anchors.leftMargin: Theme.spacing.md
        anchors.rightMargin: Theme.spacing.md
        color: Theme.colors.borderSubtle
        radius: 0.75
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
                NumberAnimation { duration: Theme.timing.normal; easing.type: Easing.OutCubic }
            }
        }
    }
}