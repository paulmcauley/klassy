cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
set(QT_MIN_VERSION "5.2.0")
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(KWIN_NAME "kwin")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
#find_package(KF5ConfigWidgets REQUIRED)

#find_package(Qt5 CONFIG REQUIRED COMPONENTS Core DBus Quick Widgets Concurrent X11Extras Test UiTools)

add_subdirectory(kdecoration2)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/decorations/kwin4_decoration_qml_breeze)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME}
        RENAME kwin4_decoration_qml_breeze.desktop)
