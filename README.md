# Better PiP

A desktop picture-in-picture application for freely sized, lockable window mirrors.

Development is in progress. This is a clean C++20 and Qt Quick application. Public release packages will be published only with explicit validation status.

## Intended experience

Choose a window, resize it to any rectangle, and lock the mirror to pass mouse input through to the application behind it. Fit, Fill, and Stretch control how the source occupies that rectangle. A global shortcut and external control window keep unlocking available.

## Build

Install Qt 6.11.2 with Qt Multimedia and Shader Tools, CMake 3.25+, Ninja, and a C++20 compiler. On Windows use an x64 Visual Studio 2022 developer terminal. Add the Qt installation to CMAKE_PREFIX_PATH.

```sh
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix dist
```

The application has no Python runtime. CI dependency installers may use their own tooling.

## Documentation

- [Architecture](docs/architecture.md)
- [Platform support](docs/platform-support.md)
- [Contributing](CONTRIBUTING.md)
- [Third-party notices](THIRD_PARTY_NOTICES.md)

## License

Application source is licensed under MIT. Dependencies retain their own licenses.
