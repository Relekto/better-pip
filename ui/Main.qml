import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "screens"

ApplicationWindow {
    id: root
    required property QtObject controller
    readonly property color ink: controller.dark ? "#edf5f0" : "#1c3027"
    readonly property color muted: controller.dark ? "#9caea4" : "#6b7f73"
    readonly property color surface: controller.dark ? "#17241d" : "#ffffff"
    readonly property color line: controller.dark ? "#2b3c32" : "#e2eae4"
    property bool settingsOpen: false
    width: 880
    height: 690
    minimumWidth: 640
    minimumHeight: 530
    visible: true
    title: "Better PiP"
    color: controller.dark ? "#101a15" : "#f3f6f2"
    font.family: Qt.platform.os === "windows" ? "Segoe UI Variable" : ""
    palette.window: color
    palette.windowText: ink
    palette.text: ink
    palette.base: surface
    palette.button: controller.dark ? "#26392d" : "#e9f0e9"
    palette.buttonText: ink
    palette.highlight: "#45bf91"
    palette.highlightedText: "#102b1f"

    onClosing: function(close) {
        if (controller.active) {
            close.accepted = false
            root.hide()
        } else controller.quit()
    }
    Connections {
        target: controller
        function onControlsRequested() {
            root.show()
            root.raise()
            root.requestActivate()
        }
    }
    PipWindow { controller: root.controller }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Image { source: "qrc:/assets/icon.svg"; sourceSize.width: 40; sourceSize.height: 40; Layout.preferredWidth: 40; Layout.preferredHeight: 40 }
            Label { text: "Better PiP"; color: root.ink; font.pixelSize: 19; font.weight: Font.DemiBold }
            Label { text: "0.1"; color: root.muted; font.pixelSize: 11 }
            Item { Layout.fillWidth: true }
            ActionButton { text: settingsOpen ? "Back to windows" : "Settings"; subdued: true; dark: controller.dark; onClicked: settingsOpen = !settingsOpen }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: noticeRow.implicitHeight + 24
            visible: controller.message.length > 0
            color: controller.dark ? "#443926" : "#fff4db"
            radius: 10
            RowLayout {
                id: noticeRow
                anchors.fill: parent
                anchors.margins: 12
                Label { text: controller.message; color: controller.dark ? "#f4d797" : "#765620"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 12 }
                ActionButton { text: "Dismiss"; subdued: true; dark: controller.dark; onClicked: controller.clearMessage() }
            }
        }
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: settingsOpen ? 1 : 0
            ColumnLayout {
                spacing: 16
                Label { text: "Keep a window in view."; color: root.ink; font.pixelSize: 29; font.weight: Font.DemiBold }
                Label { text: "Choose it. Shape it. Lock it in place."; color: root.muted; font.pixelSize: 14 }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: root.surface
                    radius: 16
                    border.color: root.line
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12
                        RowLayout {
                            Layout.fillWidth: true
                            TextField {
                                id: search
                                Layout.fillWidth: true
                                placeholderText: "Search window titles…"
                                visible: !controller.wayland
                                onTextChanged: controller.sources.filter = text
                                selectByMouse: true
                                leftPadding: 12
                                implicitHeight: 42
                                Accessible.name: "Search available windows"
                                background: Rectangle { radius: 9; color: controller.dark ? "#101a15" : "#f6f8f5"; border.color: search.activeFocus ? "#45bf91" : root.line }
                            }
                            ActionButton { text: "Refresh"; visible: !controller.wayland; subdued: true; dark: controller.dark; onClicked: controller.sources.refresh() }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            visible: !controller.wayland
                            Label { text: "AVAILABLE WINDOWS"; color: root.muted; font.pixelSize: 10; font.letterSpacing: 1.3 }
                            Item { Layout.fillWidth: true }
                            Label { text: controller.sources.count; color: root.muted; font.pixelSize: 11 }
                        }
                        ListView {
                            id: windowList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            visible: !controller.wayland
                            model: controller.sources
                            spacing: 5
                            clip: true
                            ScrollBar.vertical: ScrollBar {}
                            delegate: ItemDelegate {
                                id: row
                                required property string windowTitle
                                required property string sourceToken
                                width: ListView.view.width
                                height: 60
                                hoverEnabled: true
                                Accessible.name: "Open picture-in-picture for " + windowTitle
                                onClicked: controller.selectSource(sourceToken)
                                contentItem: RowLayout {
                                    spacing: 12
                                    Rectangle {
                                        Layout.preferredWidth: 36
                                        Layout.preferredHeight: 36
                                        radius: 9
                                        color: controller.dark ? "#304638" : "#eaf2eb"
                                        Label { anchors.centerIn: parent; text: row.windowTitle.charAt(0).toUpperCase(); color: controller.dark ? "#8be6b7" : "#33724d"; font.weight: Font.DemiBold; font.pixelSize: 15 }
                                    }
                                    Label { text: row.windowTitle; color: root.ink; Layout.fillWidth: true; elide: Text.ElideRight; font.pixelSize: 13 }
                                    Label { text: "Open PiP  ↗"; color: root.muted; font.pixelSize: 11; visible: row.hovered || row.visualFocus }
                                }
                                background: Rectangle { radius: 9; color: row.hovered || row.visualFocus ? (controller.dark ? "#22372a" : "#f0f6ef") : "transparent"; border.width: row.visualFocus ? 1 : 0; border.color: "#45bf91" }
                                ToolTip.visible: hovered
                                ToolTip.text: windowTitle
                            }
                            Label {
                                anchors.centerIn: parent
                                width: parent.width - 40
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                visible: controller.sources.count === 0
                                text: search.text.length > 0 ? "No matching windows. Try a different title." : "Open a window you want to keep in view, then refresh."
                                color: root.muted
                            }
                        }
                        ColumnLayout {
                            visible: controller.wayland
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 16
                            Item { Layout.fillHeight: true }
                            Label { text: "Choose with your desktop"; color: root.ink; font.pixelSize: 21 }
                            Label { text: "Your desktop asks which content to share. Window selection and keeping PiP above other apps depend on your desktop."; color: root.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                            ActionButton { text: "Choose a window"; primary: true; onClicked: controller.chooseWithPortal() }
                            Item { Layout.fillHeight: true }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "Local capture. No recording or uploads."; color: root.muted; font.pixelSize: 11 }
                    Item { Layout.fillWidth: true }
                    Label { text: "Fit · Fill · Stretch"; color: root.muted; font.pixelSize: 11 }
                }
            }
            SettingsPane { controller: root.controller; ink: root.ink; muted: root.muted }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 74
            visible: controller.active
            radius: 13
            color: controller.dark ? "#22392b" : "#e1f0e3"
            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Label { text: controller.locked ? "LOCKED · CLICKS PASS THROUGH" : controller.hasFrame ? "PICTURE-IN-PICTURE ACTIVE" : "CONNECTING"; color: root.muted; font.pixelSize: 10; font.letterSpacing: 0.7 }
                    Label { text: controller.sourceTitle; color: root.ink; elide: Text.ElideRight; Layout.fillWidth: true; font.pixelSize: 13 }
                }
                ActionButton { text: controller.locked ? "Unlock" : "Lock PiP"; primary: true; onClicked: controller.toggleLock() }
                ActionButton { text: "Show"; subdued: true; dark: controller.dark; onClicked: controller.showOverlay() }
                ActionButton { text: "Stop"; subdued: true; dark: controller.dark; onClicked: controller.stop() }
            }
        }
    }
}
