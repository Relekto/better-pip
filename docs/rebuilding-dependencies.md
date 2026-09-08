# Dependency sources and replacement

The release includes BetterPiP-0.1.0-dependency-sources.zip with the corresponding Qt 6.11.2 and FFmpeg 7.1.5 source archives. The source manifest records upstream URLs and SHA-256 digests. Notices and attribution files are installed beside the application. The AppImage runtime source and its dependency sources are also included.

## Qt and FFmpeg

Use the compiler for the intended target architecture. Build Qt as shared libraries from the included module archives. The shipped SDK SBOM files describe the upstream build configuration. Qt's configure instructions and module build files are in those archives.

Build FFmpeg 7.1.5 as shared libraries with its LGPL configuration, without enabling GPL or nonfree features. Point the Qt Multimedia build at that installation using FFMPEG_DIR. Build Qt Base first, followed by Qt Shader Tools, Qt Declarative, Qt SVG, Qt Multimedia, and Qt Wayland on Linux as appropriate. Install all modules into one prefix.

Set CMAKE_PREFIX_PATH to your replacement Qt prefix and build this repository with the release preset. Run CTest and the optional desktop integration tests. cmake --install stages the app and its runtime dependencies.

Dynamic libraries can be replaced in the unpacked Windows/Linux package, subject to ABI and architecture compatibility. The project does not verify or restrict replacement libraries. On macOS, rebuilding and locally signing the modified bundle may be necessary after replacing frameworks. The project does not require a vendor signature to build or run a locally signed modified application.

## AppImage

The runtime is pinned to AppImage/type2-runtime commit 75849dce7cc37e4319b633df1f116ca895c71a12. Its source includes the libfuse patch and Docker build instructions. The source bundle includes the upstream libfuse and squashfuse sources used by that recipe, plus musl, zlib, and zstd sources/notices.

To replace application libraries, extract the AppImage with --appimage-extract, change the AppDir contents, and run AppRun directly or rebuild using tools/package-linux.sh. To replace the runtime, rebuild the included runtime source with modified libraries, then pass that runtime to appimagetool's --runtime-file option.
