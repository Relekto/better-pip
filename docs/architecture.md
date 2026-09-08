# Architecture

Better PiP is implemented from the product requirements and platform documentation as an independent codebase.

## Layers

The core owns persistent preferences and state contracts. Capture services own Qt Multimedia sessions and expose sources and errors. Platform services own global shortcuts and native input behavior. The application controller coordinates them. QML renders state and invokes commands.

Windows, macOS, and X11 use Qt Multimedia's native window-capture backends and video sink without application-side CPU image copies. Wayland uses an explicit window-only XDG ScreenCast portal request and a PipeWire stream. That path copies validated frame data into an owned video frame and coalesces delivery through a bounded newest-frame mailbox.

Global shortcuts use RegisterHotKey on Windows, Carbon on macOS, XGrabKey on X11, and the GlobalShortcuts portal on Wayland. A failed replacement preserves the previous registration. Wayland registration is asynchronous and reports the desktop's actual accepted trigger.

## Ownership

QObject parent ownership is used consistently for UI services. Value members own independent services with no parent. Native registrations are released by scoped implementations. Capture stops before its output or source is destroyed.

## Lock contract

Locked means no movement, resizing, or mouse interception. The PiP's own controls are inaccessible while locked. The global shortcut, external controller, and single-instance activation provide recovery. Application restart is unlocked. Input passes to the physical window underneath.

## Privacy

Capture is local. No recording, uploads, analytics, accounts, or persisted window titles. Do not start capture automatically after restarting the application.
