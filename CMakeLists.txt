cmake_minimum_required(VERSION 3.25)
project(BetterPiP VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version")
endif()
find_package(Qt6 6.11.2 EXACT REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2 Multimedia Network Test)
qt_standard_project_setup(REQUIRES 6.11)
include(CTest)
include(cmake/CompilerOptions.cmake)
qt_add_library(pip-services STATIC
    src/core/Preferences.h src/core/Preferences.cpp
    src/core/SettingsStore.h src/core/SettingsStore.cpp
    src/capture/WindowSources.h src/capture/WindowSources.cpp
    src/capture/CaptureService.h src/capture/CaptureService.cpp
    src/platform/GlobalShortcut.h src/platform/GlobalShortcut.cpp
    src/app/SingleInstance.h src/app/SingleInstance.cpp
)
if(WIN32)
    target_sources(pip-services PRIVATE src/platform/windows/WindowsShortcut.cpp)
    target_link_libraries(pip-services PRIVATE user32)
elseif(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 20)
    target_sources(pip-services PRIVATE src/platform/macos/MacShortcut.mm)
    target_link_libraries(pip-services PRIVATE "-framework Carbon")
else()
    find_package(X11 REQUIRED)
    target_sources(pip-services PRIVATE src/platform/linux/LinuxShortcut.cpp)
    target_link_libraries(pip-services PRIVATE X11::X11)
endif()
target_include_directories(pip-services PUBLIC src)
target_link_libraries(pip-services PUBLIC Qt6::Core Qt6::Gui Qt6::Multimedia Qt6::Network)
better_pip_warnings(pip-services)
qt_add_executable(better-pip src/app/main.cpp)
target_sources(better-pip PRIVATE src/app/AppController.h src/app/AppController.cpp)
target_link_libraries(better-pip PRIVATE pip-services)
qt_add_resources(better-pip app-assets PREFIX "/" FILES assets/icon.svg)
target_link_libraries(better-pip PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Widgets Qt6::Multimedia Qt6::Network)
better_pip_warnings(better-pip)
set_target_properties(better-pip PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.relekto.better-pip"
    MACOSX_BUNDLE_BUNDLE_NAME "Better PiP"
)
qt_add_qml_module(better-pip
    URI BetterPiP
    VERSION 1.0
    QML_FILES ui/Main.qml ui/PipWindow.qml ui/components/ActionButton.qml ui/screens/SettingsPane.qml
)
if(BUILD_TESTING)
    qt_add_executable(preferences-test tests/unit/PreferencesTest.cpp)
    target_link_libraries(preferences-test PRIVATE pip-services Qt6::Test)
    add_test(NAME preferences COMMAND preferences-test)
    add_test(NAME application_startup COMMAND better-pip --smoke-test)
    set_tests_properties(application_startup PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
        TIMEOUT 15
    )
endif()
include(GNUInstallDirs)
install(TARGETS better-pip BUNDLE DESTINATION . RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
qt_generate_deploy_qml_app_script(
    TARGET better-pip
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
install(FILES LICENSE THIRD_PARTY_NOTICES.md DESTINATION licenses)
