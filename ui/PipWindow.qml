import QtQuick
import QtQuick.Controls
import QtMultimedia

Window {
    id: pip
    required property QtObject controller
    width: 640
    height: 360
    color: "#080b0c"
    title: "Better PiP · " + controller.sourceTitle
    transientParent: null
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    Component.onCompleted: controller.attachOverlay(pip, video.videoSink)
    onClosing: function (close) {
        close.accepted = false;
        controller.stop();
    }

    VideoOutput {
        id: video
        anchors.fill: parent
        fillMode: controller.scaleMode === 2 ? VideoOutput.Stretch : controller.scaleMode === 1 ? VideoOutput.PreserveAspectCrop : VideoOutput.PreserveAspectFit
        visible: controller.hasFrame
    }

    Label {
        anchors.centerIn: parent
        text: "Connecting to window…"
        color: "#c8d5d0"
        visible: !controller.hasFrame
        font.pixelSize: 14
    }

    MouseArea {
        id: moveArea
        anchors.fill: parent
        enabled: !controller.locked
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: function (mouse) {
            if (mouse.button === Qt.RightButton)
                contextMenu.popup();
            else
                pip.startSystemMove();
        }
        onDoubleClicked: controller.showControls()
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: controller.locked ? 0 : 1
        border.color: "#70ffffff"
    }

    Rectangle {
        id: toolbar
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        width: actions.implicitWidth + 16
        height: 40
        radius: 12
        color: "#ed17231f"
        visible: !controller.locked && (moveArea.containsMouse || toolbarHover.hovered || contextMenu.opened)
        HoverHandler {
            id: toolbarHover
        }
        Row {
            id: actions
            anchors.centerIn: parent
            spacing: 4
            Button {
                text: "Lock"
                onClicked: controller.toggleLock()
                ToolTip.visible: hovered
                ToolTip.text: "Clicks pass through. Unlock with " + controller.shortcut
            }
            Button {
                text: "Controls"
                onClicked: controller.showControls()
            }
            Button {
                text: "×"
                onClicked: controller.stop()
                Accessible.name: "Close picture-in-picture"
            }
        }
    }

    Repeater {
        model: [
            {
                edge: Qt.LeftEdge,
                cursor: Qt.SizeHorCursor,
                left: true,
                right: false,
                top: false,
                bottom: false
            },
            {
                edge: Qt.RightEdge,
                cursor: Qt.SizeHorCursor,
                left: false,
                right: true,
                top: false,
                bottom: false
            },
            {
                edge: Qt.TopEdge,
                cursor: Qt.SizeVerCursor,
                left: false,
                right: false,
                top: true,
                bottom: false
            },
            {
                edge: Qt.BottomEdge,
                cursor: Qt.SizeVerCursor,
                left: false,
                right: false,
                top: false,
                bottom: true
            },
            {
                edge: Qt.TopEdge | Qt.LeftEdge,
                cursor: Qt.SizeFDiagCursor,
                left: true,
                right: false,
                top: true,
                bottom: false
            },
            {
                edge: Qt.TopEdge | Qt.RightEdge,
                cursor: Qt.SizeBDiagCursor,
                left: false,
                right: true,
                top: true,
                bottom: false
            },
            {
                edge: Qt.BottomEdge | Qt.LeftEdge,
                cursor: Qt.SizeBDiagCursor,
                left: true,
                right: false,
                top: false,
                bottom: true
            },
            {
                edge: Qt.BottomEdge | Qt.RightEdge,
                cursor: Qt.SizeFDiagCursor,
                left: false,
                right: true,
                top: false,
                bottom: true
            }
        ]
        MouseArea {
            required property var modelData
            readonly property bool corner: (modelData.left || modelData.right) && (modelData.top || modelData.bottom)
            x: modelData.right ? pip.width - width : 0
            y: modelData.bottom ? pip.height - height : 0
            width: corner ? 12 : ((modelData.left || modelData.right) ? 6 : pip.width)
            height: corner ? 12 : ((modelData.top || modelData.bottom) ? 6 : pip.height)
            enabled: !controller.locked
            cursorShape: modelData.cursor
            onPressed: pip.startSystemResize(modelData.edge)
        }
    }

    Menu {
        id: contextMenu
        MenuItem {
            text: "Choose another window"
            onTriggered: controller.showControls()
        }
        MenuSeparator {}
        MenuItem {
            text: "Fit · show whole window"
            checkable: true
            checked: controller.scaleMode === 0
            onTriggered: controller.scaleMode = 0
        }
        MenuItem {
            text: "Fill · crop to frame"
            checkable: true
            checked: controller.scaleMode === 1
            onTriggered: controller.scaleMode = 1
        }
        MenuItem {
            text: "Stretch · fill freely"
            checkable: true
            checked: controller.scaleMode === 2
            onTriggered: controller.scaleMode = 2
        }
        MenuItem {
            text: "Keep source proportions"
            checkable: true
            checked: controller.preserveAspect
            onTriggered: controller.preserveAspect = !controller.preserveAspect
        }
        MenuSeparator {}
        MenuItem {
            text: "Lock and pass clicks through"
            onTriggered: controller.toggleLock()
        }
        MenuItem {
            text: "Settings and controls"
            onTriggered: controller.showControls()
        }
        MenuItem {
            text: "Close PiP"
            onTriggered: controller.stop()
        }
    }
}
