# Documentation media

This optional C++ tool loads the application's actual QML and controller. A separate process supplies an animated geometric window. Window enumeration is filtered and checked against that exact title before exporting any frame. No desktop screenshot or personal window content is exported.

This tool produces the app screenshots and the earlier geometric demonstration. The current README video is a separate stock-footage edit; see its [credits](../../docs/media/CREDITS.md).

Build in the same Qt developer environment as the app:

```sh
cmake --preset release -DBETTER_PIP_BUILD_STUDIO=ON
cmake --build --preset release --target pip-studio
./build/release/pip-studio out/presentation
```

Run in an unlocked Windows desktop session with enough room for the source and app windows. Qt's SDK libraries must be on the runtime path. The tool uses transient preferences, leaves normal app settings alone, and closes its child source after capture. It is neither installed nor included in released application packages.

The output contains 450 frames and three presentation screenshots. Frames contain real application pixels with separate editorial captions. Playback is an automated 15-second sequence at 30 fps; export can take longer than playback. The sequence exercises actual capture, scaling, geometry and native locking. It does not inject mouse input or demonstrate a physical click reaching another application. Native click-through is checked separately by the desktop integration test. Shortcut registration is disabled in the isolated presentation controller.

Encode with an independently installed FFmpeg CLI:

```sh
ffmpeg -framerate 30 -i out/presentation/frames/%04d.png -c:v libx264 -crf 20 -pix_fmt yuv420p -movflags +faststart out/presentation/geometric-demo.mp4
ffmpeg -i out/presentation/geometric-demo.mp4 -filter_complex "fps=12,scale=768:-1:flags=lanczos,split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=bayer" -loop 0 out/presentation/geometric-demo.gif
```

Inspect all scene boundaries and the three screenshots before publishing. Keep the README's media version note accurate when the source appearance differs from the current binary release.
