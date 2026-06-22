// src/shell/qml/Theme.qml
pragma Singleton

import QtQuick

QtObject {
    readonly property SystemPalette palette: SystemPalette {}

    readonly property QtObject colors: QtObject {
        readonly property color background: palette.window
        readonly property color surface: palette.window
        readonly property color surfaceRaised: palette.alternateBase
        readonly property color surfaceSoft: palette.base
        readonly property color text: palette.text
        readonly property color textSecondary: palette.mid
        readonly property color accent: palette.highlight
        readonly property color accentLight: Qt.lighter(palette.highlight, 1.45)
        readonly property color success: "#63A85A"
        readonly property color danger: "#E24B4A"
        readonly property color border: palette.midlight
        readonly property color borderStrong: palette.mid
        readonly property color overlay: Qt.rgba(0.0, 0.0, 0.0, 0.14)
    }

    readonly property QtObject spacing: QtObject {
        readonly property int xs: 4
        readonly property int sm: 8
        readonly property int md: 12
        readonly property int lg: 16
        readonly property int xl: 24
        readonly property int xxl: 32
    }

    readonly property QtObject fontSize: QtObject {
        readonly property int xs: 11
        readonly property int sm: 12
        readonly property int md: 14
        readonly property int lg: 16
        readonly property int xl: 18
        readonly property int xxl: 22
    }

    readonly property QtObject radius: QtObject {
        readonly property int sm: 4
        readonly property int md: 8
        readonly property int lg: 12
    }

    readonly property QtObject timing: QtObject {
        readonly property int fast: 150
        readonly property int normal: 300
        readonly property int slow: 500
    }

    readonly property QtObject fonts: QtObject {
        readonly property font defaultFont: Qt.font({family: Qt.application.font.family, pixelSize: fontSize.md, weight: Font.Normal})
        readonly property font mono: Qt.font({family: "Courier New", pixelSize: fontSize.md})
    }
}
