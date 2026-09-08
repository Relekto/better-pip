# Validation and remaining qualification

## Automated coverage

- Preferences: validation, extreme geometry/aspect values, atomic settings round trips, malformed and future-version data, and monitor-bound recovery.
- Startup: real QML loading under an offscreen platform; QML warnings fail the smoke test.
- Windows desktop: a separate animated fixture process, captured-content assertion, independent dimensions, aspect mode, geometry freezing, native WindowFromPoint hit testing, 100 lock/unlock cycles, real Qt Quick video rendering across lock changes, source closure, and hotkey conflicts preserving the previous binding.
- X11 desktop: capture from a separate fixture under Xvfb/Openbox, content validation, geometry/state behavior, real rendering, source closure, and native shortcut conflict coverage.
- Packaging: startup with SDK plugin/library search variables removed, C++ runtime DLL presence on Windows, and macOS bundle signature verification.
- CI static analysis: Clang's path-sensitive analyzers on Linux C++ translation units. The locally bundled Windows clang-tidy executable crashed before analysis; it is not counted as a successful check.

Only the application's own test fixture is captured during automated tests. Tests do not inspect unrelated user windows or save their contents.

## Before a stable release

Qualify real Windows 11, macOS Intel/ARM, and named Wayland desktops. Exercise screen permissions and revocation, sleep/resume, source minimization, mixed DPI, monitor removal, multi-GPU changes, and Spaces/workspaces. Confirm actual left/right/wheel input delivery under the locked overlay on each desktop; Windows automated coverage currently checks native hit testing.

Measure sustained CPU/GPU/memory usage and capture-to-display latency at 1080p30/60 on named hardware. Quantitative performance, HDR color accuracy, accessibility review, and a visual design review are not yet completed. No claim of zero overhead or production qualification is made for 0.1.0.
