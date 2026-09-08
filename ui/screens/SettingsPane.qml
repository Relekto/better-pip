import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

ScrollView {
    id: pane
    required property QtObject controller
    required property color ink
    required property color muted
    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: pane.availableWidth
        spacing: 20
        Label {
            text: "Make it yours"
            color: pane.ink
            font.pixelSize: 24
            font.weight: Font.DemiBold
        }
        Label {
            text: "Shortcuts, appearance, and the way your picture fits."
            color: pane.muted
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Label {
            text: "LOCK / UNLOCK SHORTCUT"
            color: pane.muted
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: shortcutField
                Layout.fillWidth: true
                text: controller.shortcut
                placeholderText: "Click here and press a shortcut"
                readOnly: true
                selectByMouse: true
                Accessible.name: "Record global lock shortcut"
                Keys.onPressed: function (event) {
                    if ([Qt.Key_Control, Qt.Key_Shift, Qt.Key_Alt, Qt.Key_Meta].indexOf(event.key) >= 0) {
                        event.accepted = true;
                        return;
                    }
                    if (event.key === Qt.Key_Tab || event.key === Qt.Key_Escape)
                        return;
                    let parts = [];
                    if (event.modifiers & Qt.ControlModifier)
                        parts.push("Ctrl");
                    if (event.modifiers & Qt.AltModifier)
                        parts.push("Alt");
                    if (event.modifiers & Qt.ShiftModifier)
                        parts.push("Shift");
                    if (event.modifiers & Qt.MetaModifier)
                        parts.push("Meta");
                    if (event.key >= Qt.Key_F1 && event.key <= Qt.Key_F24)
                        parts.push("F" + (event.key - Qt.Key_F1 + 1));
                    else if (event.key === Qt.Key_Space)
                        parts.push("Space");
                    else if ((event.key >= Qt.Key_A && event.key <= Qt.Key_Z) || (event.key >= Qt.Key_0 && event.key <= Qt.Key_9))
                        parts.push(String.fromCharCode(event.key));
                    else {
                        event.accepted = true;
                        return;
                    }
                    text = parts.join("+");
                    event.accepted = true;
                }
            }
            ActionButton {
                text: "Save shortcut"
                dark: controller.dark
                onClicked: controller.setShortcut(shortcutField.text)
            }
        }
        Label {
            text: controller.shortcutStatus
            color: pane.muted
            font.pixelSize: 12
        }
        Label {
            text: "When locked, the PiP passes every click through. Use this shortcut, the tray menu, or reopen Better PiP to unlock."
            color: pane.muted
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Label {
            text: "APPEARANCE"
            color: pane.muted
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        ComboBox {
            model: ["Follow system", "Light", "Dark"]
            currentIndex: controller.theme
            onActivated: controller.theme = currentIndex
            Layout.preferredWidth: 220
            Accessible.name: "Application theme"
        }
        Label {
            text: "PICTURE MODE"
            color: pane.muted
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        ComboBox {
            model: ["Fit — whole window", "Fill — crop to frame", "Stretch — any proportions"]
            currentIndex: controller.scaleMode
            onActivated: controller.scaleMode = currentIndex
            Layout.preferredWidth: 280
            Accessible.name: "Picture scaling mode"
        }
        CheckBox {
            text: "Keep source proportions when resizing"
            checked: controller.preserveAspect
            onToggled: controller.preserveAspect = checked
        }
        Label {
            text: "OPACITY · " + controller.opacity + "%"
            color: pane.muted
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        Slider {
            from: 20
            to: 100
            stepSize: 1
            value: controller.opacity
            onMoved: controller.opacity = value
            Layout.fillWidth: true
            Accessible.name: "PiP opacity"
        }
        Label {
            text: "EXACT SIZE"
            color: pane.muted
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        RowLayout {
            SpinBox {
                id: widthInput
                from: 160
                to: 8192
                value: controller.pipWidth
                editable: true
                Accessible.name: "PiP width"
            }
            Label {
                text: "×"
                color: pane.muted
            }
            SpinBox {
                id: heightInput
                from: 90
                to: 8192
                value: controller.pipHeight
                editable: true
                Accessible.name: "PiP height"
            }
            ActionButton {
                text: "Apply"
                dark: controller.dark
                enabled: !controller.locked
                onClicked: controller.resizePip(widthInput.value, heightInput.value)
            }
        }
        ActionButton {
            text: "Reset preferences"
            subdued: true
            dark: controller.dark
            onClicked: controller.resetPreferences()
        }
        Item {
            Layout.preferredHeight: 8
        }
    }
}
