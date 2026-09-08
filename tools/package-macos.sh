set -eu
architecture=$(uname -m)
mkdir -p out/release out/dmg
ditto dist/better-pip.app "out/dmg/Better PiP.app"
ln -sfn /Applications out/dmg/Applications
codesign --force --deep --sign - "out/dmg/Better PiP.app"
codesign --verify --deep --strict "out/dmg/Better PiP.app"
env -u DYLD_LIBRARY_PATH -u QT_PLUGIN_PATH -u QML2_IMPORT_PATH -u QML_IMPORT_PATH QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software "out/dmg/Better PiP.app/Contents/MacOS/better-pip" --smoke-test
hdiutil create -volname "Better PiP" -srcfolder out/dmg -ov -format UDZO "out/release/BetterPiP-0.1.0-macos-$architecture.dmg"
(cd out/release && shasum -a 256 ./*.dmg > "SHA256SUMS-macos-$architecture.txt")
