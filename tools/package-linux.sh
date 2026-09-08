set -eu
mkdir -p out/release out/AppDir/usr out/package-tools
cp -a dist/. out/AppDir/usr/
cp packaging/linux/AppRun out/AppDir/AppRun
cp packaging/linux/io.github.relekto.better-pip.desktop out/AppDir/
cp assets/icon.svg out/AppDir/better-pip.svg
chmod +x out/AppDir/AppRun
curl -fLsS --retry 3 -o out/package-tools/appimagetool.AppImage https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
curl -fLsS --retry 3 -o out/package-tools/runtime-x86_64 https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64
printf '%s\n' 'a6d71e2b6cd66f8e8d16c37ad164658985e0cf5fcaa950c90a482890cb9d13e0  out/package-tools/appimagetool.AppImage' '1cc49bcf1e2ccd593c379adb17c9f85a36d619088296504de95b1d06215aebbf  out/package-tools/runtime-x86_64' | sha256sum -c -
chmod +x out/package-tools/appimagetool.AppImage
ARCH=x86_64 APPIMAGE_EXTRACT_AND_RUN=1 out/package-tools/appimagetool.AppImage --runtime-file out/package-tools/runtime-x86_64 out/AppDir out/release/BetterPiP-0.1.0-linux-x64.AppImage
tar -czf out/release/BetterPiP-0.1.0-linux-x64.tar.gz -C out/AppDir .
env -u LD_LIBRARY_PATH -u QT_PLUGIN_PATH -u QML2_IMPORT_PATH -u QML_IMPORT_PATH QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software APPIMAGE_EXTRACT_AND_RUN=1 out/release/BetterPiP-0.1.0-linux-x64.AppImage --smoke-test
(cd out/release && sha256sum ./*.AppImage ./*.tar.gz > SHA256SUMS-linux.txt)
