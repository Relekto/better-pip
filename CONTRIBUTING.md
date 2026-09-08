# Contributing

Use a clean checkout, the pinned Qt version, and the documented CMake presets. Format authored C++ using the repository clang-format configuration and run CTest before submitting a change.

Keep changes focused. Use explicit ownership and RAII for resources, preserve UI-thread affinity, and surface errors as actionable state. Do not introduce unbounded frame queues, platform logic in QML, or private Qt APIs.

Authored source and build configuration contain no comments. Explain decisions and contracts in Markdown, with meaningful names and focused tests in code. Preserve dependency license notices.

Use descriptive commit subjects such as `feat(overlay): add stretch sizing`. Each completed milestone is validated, committed, and pushed. Never include credentials, local capture content, build output, or personal filesystem paths in commits.

Runtime platform support requires desktop testing; a green cross-platform build is not a claim that capture and global shortcuts have been tested on that platform.
