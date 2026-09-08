# Architecture

Better PiP is implemented from the product requirements and platform documentation as an independent codebase.

## Layers

The core owns persistent preferences and state contracts. Capture services own Qt Multimedia sessions and expose sources and errors. Platform services own global shortcuts and native input behavior. The application controller coordinates them. QML renders state and invokes commands.

Use Qt's maintained window-capture backend first and validate it before introducing a custom graphics pipeline. This replaces the initial proposal to write all native capture backends ourselves: it reduces resource-lifetime and synchronization code while keeping native platform integration available behind services. Frame delivery remains in Qt Multimedia and its video sink, without CPU image copies in application code.

## Ownership

QObject parent ownership is used consistently for UI services. Value members own independent services with no parent. Native registrations are released by scoped implementations. Capture stops before its output or source is destroyed.

## Lock contract

Locked means no movement, resizing, or mouse interception. The PiP's own controls are inaccessible while locked. The global shortcut, external controller, and single-instance activation provide recovery. Application restart is unlocked. Input passes to the physical window underneath.

## Privacy

Capture is local. No recording, uploads, analytics, accounts, or persisted window titles. Do not start capture automatically after restarting the application.
