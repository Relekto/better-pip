# Platform support

Targets are Windows 10 1903+ and Windows 11 x64, macOS 13+ Intel/Apple Silicon, and Linux x64.

| Environment | Build | Capture and lock runtime |
| --- | --- | --- |
| Windows x64 | Pending | Pending |
| macOS Intel | Pending | Pending |
| macOS Apple Silicon | Pending | Pending |
| Linux X11 | Pending | Pending |
| Linux Wayland | Pending | Experimental; portal and compositor dependent |

Qt 6.11.2 is pinned. Qt upgrades must retain Windows 10 compatibility. Window capture requires Qt Multimedia's FFmpeg backend. Some backends deliver frames only when source content changes, so absence of frames alone does not indicate failure.

Protected content and secure desktops are not universally capturable. Minimized/off-screen window behavior differs by platform. Standard Wayland does not provide a universal window-title list or always-on-top request. Unsupported capabilities must be visible rather than silently claimed.

Sources: https://doc.qt.io/qt-6/qwindowcapture.html and https://doc.qt.io/qt-6/qscreencapture.html.
