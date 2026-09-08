import QtQuick

QtObject {
    property bool dark: false
    readonly property color background: dark ? "#111318" : "#f4f6fa"
    readonly property color surface: dark ? "#1a1d24" : "#ffffff"
    readonly property color inset: dark ? "#14171d" : "#f7f8fb"
    readonly property color ink: dark ? "#f0f2f7" : "#202633"
    readonly property color muted: dark ? "#a2aaba" : "#657087"
    readonly property color line: dark ? "#303642" : "#dce2ec"
    readonly property color button: dark ? "#272d38" : "#e9edf5"
    readonly property color hover: dark ? "#333c4b" : "#dde5f2"
    readonly property color accent: dark ? "#79a8ff" : "#2563d9"
    readonly property color accentHover: dark ? "#9bbeff" : "#1d55bf"
    readonly property color accentPressed: dark ? "#6095f0" : "#17479f"
    readonly property color onAccent: dark ? "#101c34" : "#ffffff"
    readonly property color accentSurface: dark ? "#202f48" : "#e8effc"
    readonly property color accentInk: dark ? "#a4c4ff" : "#2557ad"
    readonly property color disabled: dark ? "#778092" : "#8490a2"
    readonly property color warningSurface: dark ? "#443926" : "#fff4db"
    readonly property color warningInk: dark ? "#f4d797" : "#765620"
}
