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
qt_add_executable(better-pip src/app/main.cpp)
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
    QML_FILES ui/Main.qml
)
if(BUILD_TESTING)
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
