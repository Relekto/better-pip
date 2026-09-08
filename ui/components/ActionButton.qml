import QtQuick
import QtQuick.Controls

Button {
    id: control
    property bool primary: false
    property bool subdued: false
    property bool dark: false
    implicitHeight: 40
    leftPadding: 16
    rightPadding: 16
    hoverEnabled: true
    font.pixelSize: 13
    font.weight: Font.DemiBold
    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? "#7e8985" : control.primary ? "#10291f" : control.dark ? "#edf5f1" : "#253b32"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 9
        color: control.primary ? (control.down ? "#40ba94" : control.hovered ? "#75e6be" : "#55d5a9")
              : control.subdued ? (control.hovered ? (control.dark ? "#263930" : "#e3ebe6") : "transparent")
              : control.dark ? (control.hovered ? "#314238" : "#233229")
              : control.hovered ? "#e1ebe5" : "#edf2ee"
        border.width: control.visualFocus ? 2 : 0
        border.color: "#24a77b"
        opacity: control.enabled ? 1 : 0.55
    }
}
