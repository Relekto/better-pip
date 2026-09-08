# Platform support

Targets are Windows 10 1903+ and Windows 11 x64, macOS 13+ Intel/Apple Silicon, and Linux x64. Version 0.1.0 is a preview.

| Environment | Build and startup | Desktop validation |
| --- | --- | --- |
| Windows x64 | CI and local build/startup | Windows 10 22H2: real capture/rendering, independent resize, 100 lock cycles, click-through hit testing, source closure, and hotkey conflict recovery passed |
| Windows 11 x64 | Same Windows binary | Separate Windows 11 desktop qualification pending |
| macOS Intel | CI build and offscreen startup | Screen Recording permission, capture, Spaces, and hotkeys require real desktop qualification |
| macOS Apple Silicon | CI build and offscreen startup | Screen Recording permission, capture, Spaces, and hotkeys require real desktop qualification |
| Linux X11 | CI build and startup | Xvfb/Openbox capture, resize, rendering through lock/unlock, and source closure passed; physical input qualification pending |
| Linux Wayland | Linux CI compiles portal/PipeWire paths | Experimental; consent, hotkeys, and compositor behavior require real desktop qualification |

## Constraints

Qt 6.11.2 is pinned. Qt upgrades must retain Windows 10 compatibility. Window capture uses Qt Multimedia's FFmpeg backend on Windows, macOS, and X11. Some backends deliver frames only when source content changes; an idle source is not automatically a capture failure.

Protected content and secure desktops are not universally capturable. Minimized and off-screen behavior differs by backend. Keep the source restored. X11 capture can be affected by overlapping windows and the window manager/compositor; test your desktop. Bare X11 sessions must export XDG_SESSION_TYPE=x11 for Qt's capture backend.

Wayland uses a window-only ScreenCast portal request and PipeWire. The desktop owns the permission chooser, so the application cannot provide an unrestricted searchable list of native Wayland window titles. Standard Wayland has no universal always-on-top request. Global shortcuts require a desktop implementation of the GlobalShortcuts portal; rejected or unavailable bindings leave the external control window available for unlocking. No Wayland desktop is certified for this preview.

Restarting starts unlocked and never restores a capture automatically. Reopening the application unlocks and reveals its control window. If the single-instance recovery channel cannot start, the application reports that failure instead of opening an unrecoverable overlay.

Windows packages are unsigned. macOS bundles are ad-hoc signed and verified during packaging, without Developer ID signing or notarization. Startup checks isolate packages from Qt SDK search paths; this is not a clean physical-machine certification.

See [Qt window capture](https://doc.qt.io/qt-6/qwindowcapture.html), [ScreenCast portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.ScreenCast.html), and [GlobalShortcuts portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html).
