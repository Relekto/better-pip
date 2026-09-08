# Third-party notices

Better PiP is MIT licensed. Dependency licenses remain in force.

Better PiP uses Qt 6.11.2, copyright The Qt Company Ltd. and other contributors, under the LGPLv3 terms for its shared Qt libraries. Modules include Qt Base, Declarative, Multimedia, SVG, Shader Tools, and Wayland on Linux. The full GPLv3 and LGPLv3 texts, upstream copyright notices, bundled-component attributions, and SDK SBOM files are included in the package's licenses directory. Settings → Open licenses opens that directory.

Qt Multimedia uses FFmpeg 7.1.5 under LGPLv2.1 or later. The official Qt binaries identify this as an LGPL build. FFmpeg notices and license texts accompany the application.

Windows packages include Microsoft Visual C++ and DirectX redistributable components under their Microsoft terms. DirectX Shader Compiler 1.8.2502 is supplied by the Qt SDK; its Microsoft, LLVM, and third-party notices are included. The installer uses NSIS under its upstream licenses; NSIS notices are also included. These components are not relicensed by the application's MIT license.

The Linux AppImage includes the MIT-licensed AppImage type-2 runtime and its statically linked musl, libfuse, squashfuse, zlib, and zstd components. Their notices and source archives accompany this release. Linux system libraries such as PipeWire, X11, and the system C library retain their upstream licenses.

Download **BetterPiP-0.1.0-dependency-sources.zip** alongside the binaries from [the release](https://github.com/Relekto/better-pip/releases/tag/v0.1.0). It contains corresponding Qt and FFmpeg sources, the pinned AppImage runtime and dependency sources, and a SHA-256 source manifest. The application source and build scripts are available from the same tag. See [rebuilding and replacing dependencies](docs/rebuilding-dependencies.md).

No application restriction prevents modification, library replacement, or reverse engineering to debug modifications. Package signatures may need to be renewed locally after modifying a macOS bundle.

Upstream references: [Qt licensing](https://doc.qt.io/qt-6/licensing.html), [Qt's LGPL text](https://doc.qt.io/qt-6/lgpl.html), and [FFmpeg attribution](https://doc.qt.io/qt-6/qtmultimedia-attribution-ffmpeg.html).
