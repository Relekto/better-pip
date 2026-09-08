import QtQuick
import QtQuick.Controls

Button {
    id: control
    property bool primary: false
    property bool subdued: false
    property bool dark: false
    AppPalette {
        id: theme
        dark: control.dark
    }
    implicitHeight: 40
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true
    font.pixelSize: 13
    font.weight: Font.DemiBold
    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? theme.disabled : control.primary ? theme.onAccent : theme.ink
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 9
        color: control.primary ? (control.down ? theme.accentPressed : control.hovered ? theme.accentHover : theme.accent) : control.subdued ? (control.hovered ? theme.hover : "transparent") : control.hovered ? theme.hover : theme.button
        border.width: control.visualFocus ? 2 : 0
        border.color: theme.accent
        opacity: control.enabled ? 1 : 0.55
    }
}
