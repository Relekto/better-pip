import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"
import "screens"

ApplicationWindow {
    id: root
    required property QtObject controller
    readonly property color ink: theme.ink
    readonly property color muted: theme.muted
    readonly property color surface: theme.surface
    readonly property color line: theme.line
    AppPalette {
        id: theme
        dark: controller.dark
    }
    property bool settingsOpen: false
    width: 880
    height: 690
    minimumWidth: 640
    minimumHeight: 530
    visible: true
    title: "Better PiP"
    color: theme.background
    font.family: Qt.platform.os === "windows" ? "Segoe UI Variable" : ""
    palette.window: color
    palette.windowText: ink
    palette.text: ink
    palette.base: surface
    palette.button: theme.button
    palette.buttonText: ink
    palette.highlight: theme.accent
    palette.highlightedText: theme.onAccent

    onClosing: function (close) {
        if (controller.active) {
            close.accepted = false;
            root.hide();
        } else
            controller.quit();
    }
    Connections {
        target: controller
        function onControlsRequested() {
            root.show();
            root.raise();
            root.requestActivate();
        }
    }
    PipWindow {
        controller: root.controller
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Image {
                source: "qrc:/assets/icon.svg"
                sourceSize.width: 40
                sourceSize.height: 40
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
            }
            Label {
                text: "Better PiP"
                color: root.ink
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            Label {
                text: "0.1"
                color: root.muted
                font.pixelSize: 11
            }
            Item {
                Layout.fillWidth: true
            }
            ActionButton {
                text: settingsOpen ? "Back to windows" : "Settings"
                subdued: true
                dark: controller.dark
                onClicked: settingsOpen = !settingsOpen
            }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: noticeRow.implicitHeight + 24
            visible: controller.message.length > 0
            color: theme.warningSurface
            radius: 10
            RowLayout {
                id: noticeRow
                anchors.fill: parent
                anchors.margins: 12
                Label {
                    text: controller.message
                    textFormat: Text.PlainText
                    color: theme.warningInk
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    font.pixelSize: 12
                }
                ActionButton {
                    text: "Dismiss"
                    subdued: true
                    dark: controller.dark
                    onClicked: controller.clearMessage()
                }
            }
        }
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: settingsOpen ? 1 : 0
            ColumnLayout {
                spacing: 16
                Label {
                    text: "Keep a window in view."
                    color: root.ink
                    font.pixelSize: 29
                    font.weight: Font.DemiBold
                }
                Label {
                    text: "Choose it. Shape it. Lock it in place."
                    color: root.muted
                    font.pixelSize: 14
                }
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
                                background: Rectangle {
                                    radius: 9
                                    color: theme.inset
                                    border.color: search.activeFocus ? theme.accent : root.line
                                }
                            }
                            ActionButton {
                                text: "Refresh"
                                visible: !controller.wayland
                                subdued: true
                                dark: controller.dark
                                onClicked: controller.sources.refresh()
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            visible: !controller.wayland
                            Label {
                                text: "AVAILABLE WINDOWS"
                                color: root.muted
                                font.pixelSize: 10
                                font.letterSpacing: 1.3
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                            Label {
                                text: controller.sources.count
                                color: root.muted
                                font.pixelSize: 11
                            }
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
                                        color: theme.accentSurface
                                        Label {
                                            anchors.centerIn: parent
                                            text: row.windowTitle.charAt(0).toUpperCase()
                                            color: theme.accentInk
                                            font.weight: Font.DemiBold
                                            font.pixelSize: 15
                                        }
                                    }
                                    Label {
                                        text: row.windowTitle
                                        textFormat: Text.PlainText
                                        color: root.ink
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                        font.pixelSize: 13
                                    }
                                    Label {
                                        text: "Open PiP  ↗"
                                        color: root.muted
                                        font.pixelSize: 11
                                        visible: row.hovered || row.visualFocus
                                    }
                                }
                                background: Rectangle {
                                    radius: 9
                                    color: row.hovered || row.visualFocus ? theme.accentSurface : "transparent"
                                    border.width: row.visualFocus ? 1 : 0
                                    border.color: theme.accent
                                }
                                ToolTip {
                                    id: sourceTip
                                    visible: row.hovered
                                    text: row.windowTitle
                                    contentItem: Text {
                                        text: sourceTip.text
                                        textFormat: Text.PlainText
                                        font: sourceTip.font
                                        color: sourceTip.palette.toolTipText
                                    }
                                }
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
                            Item {
                                Layout.fillHeight: true
                            }
                            Label {
                                text: "Choose with your desktop"
                                color: root.ink
                                font.pixelSize: 21
                            }
                            Label {
                                text: "Your desktop asks which content to share. Window selection and keeping PiP above other apps depend on your desktop."
                                color: root.muted
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }
                            ActionButton {
                                text: "Choose a window"
                                primary: true
                                dark: controller.dark
                                onClicked: controller.chooseWithPortal()
                            }
                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "Local capture. No recording or uploads."
                        color: root.muted
                        font.pixelSize: 11
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Label {
                        text: "Fit · Fill · Stretch"
                        color: root.muted
                        font.pixelSize: 11
                    }
                }
            }
            SettingsPane {
                controller: root.controller
                ink: root.ink
                muted: root.muted
            }
        }
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 74
            visible: controller.active
            radius: 13
            color: theme.accentSurface
            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Label {
                        text: controller.locked ? "LOCKED · CLICKS PASS THROUGH" : controller.hasFrame ? "PICTURE-IN-PICTURE ACTIVE" : "CONNECTING"
                        color: root.muted
                        font.pixelSize: 10
                        font.letterSpacing: 0.7
                    }
                    Label {
                        text: controller.sourceTitle
                        textFormat: Text.PlainText
                        color: root.ink
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        font.pixelSize: 13
                    }
                }
                ActionButton {
                    text: controller.locked ? "Unlock" : "Lock PiP"
                    primary: true
                    dark: controller.dark
                    onClicked: controller.toggleLock()
                }
                ActionButton {
                    text: "Show"
                    subdued: true
                    dark: controller.dark
                    onClicked: controller.showOverlay()
                }
                ActionButton {
                    text: "Stop"
                    subdued: true
                    dark: controller.dark
                    onClicked: controller.stop()
                }
            }
        }
    }
}
