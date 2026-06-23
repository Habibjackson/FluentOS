// src/shell/qml/Theme.qml
pragma Singleton
import QtQuick
import Fluent // Imports the module where SystemState is registered

QtObject {
    id: themeRoot

    // ==========================================
    // GLOBAL THEME STATE
    // ==========================================
    // Binds directly to the integrated SystemState settings engine
    readonly property bool isDark: SystemState.isDark

    // ==========================================
    // DYNAMIC COLORS
    // ==========================================
    readonly property QtObject colors: QtObject {
        // Core Backgrounds
        readonly property color background: themeRoot.isDark ? "#1C1C1E" : "#F2F2F7"
        readonly property color surface: themeRoot.isDark ? "#2C2C2E" : "#FFFFFF"
        readonly property color surfaceRaised: themeRoot.isDark ? "#3A3A3C" : "#F8F8F8"
        readonly property color surfaceSoft: themeRoot.isDark ? "#48484A" : "#E5E5EA"
        
        // Typography
        readonly property color text: themeRoot.isDark ? "#FFFFFF" : "#000000"
        readonly property color textSecondary: themeRoot.isDark ? "#EBEBF5" : "#3C3C43"
        readonly property color textDisabled: themeRoot.isDark ? Qt.rgba(1, 1, 1, 0.38) : Qt.rgba(0, 0, 0, 0.38)
        
        // Brand & Accent (Now bound directly to SystemState.customAccent!)
        readonly property color accent: SystemState.customAccent
        readonly property color accentLight: Qt.lighter(accent, 1.3)
        
        // Semantic Colors
        readonly property color success: themeRoot.isDark ? "#32D74B" : "#34C759"
        readonly property color warning: themeRoot.isDark ? "#FFD60A" : "#FFCC00"
        readonly property color danger: themeRoot.isDark ? "#FF453A" : "#FF3B30"
        
        // Interactive States (For HoverHandler / TapHandler)
        readonly property color hoverOverlay: themeRoot.isDark ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.05)
        readonly property color pressedOverlay: themeRoot.isDark ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(0, 0, 0, 0.10)
        
        // Borders & Shadows
        readonly property color border: themeRoot.isDark ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(0, 0, 0, 0.15)
        readonly property color borderSubtle: themeRoot.isDark ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.05)
        readonly property color shadow: themeRoot.isDark ? Qt.rgba(0, 0, 0, 0.50) : Qt.rgba(0, 0, 0, 0.15)
    }

    // ==========================================
    // GLASSMORPHISM ALPHA SPECS
    // ==========================================
    readonly property QtObject alpha: QtObject {
        readonly property real panelBackground: themeRoot.isDark ? 0.70 : 0.85
        readonly property int blurRadius: 32
    }

    // ==========================================
    // SHADOW ELEVATIONS
    // ==========================================
    readonly property QtObject elevation: QtObject {
        readonly property QtObject floatingBar: QtObject {
            readonly property int blur: 16
            readonly property int yOffset: 4
        }
        readonly property QtObject popoutMenu: QtObject {
            readonly property int blur: 32
            readonly property int yOffset: 12
        }
    }

    // ==========================================
    // SIZING & METRICS
    // ==========================================
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
    
    readonly property QtObject fontWeight: QtObject {
        readonly property int regular: Font.Normal
        readonly property int medium: Font.Medium
        readonly property int bold: Font.Bold
    }

    readonly property QtObject radius: QtObject {
        readonly property int sm: 4
        readonly property int md: 8
        readonly property int lg: 12
        readonly property int pill: 22
    }

    readonly property QtObject timing: QtObject {
        readonly property int fast: 150
        readonly property int normal: 300
        readonly property int slow: 500
    }

    // Example of adding custom animation curves inside Theme.qml
    readonly property QtObject curves: QtObject {
        readonly property var springEasing: Easing.OutBack
        readonly property var fluidEasing: Easing.InOutCubic
    }
}