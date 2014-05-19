project(breeze)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include(WriteBasicConfigVersionFile)
include(FeatureSummary)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(GenerateExportHeader)

add_subdirectory(cursors)
add_subdirectory(windec)
add_subdirectory(widgetstyles)
add_subdirectory(kde4breeze)

INSTALL(FILES colors/Breeze.colors DESTINATION ${DATA_INSTALL_DIR}/color-schemes/)

#KDE4 stuff
#FIXME: this works only if kde4 is installed in the same prefix
INSTALL(FILES colors/Breeze.colors DESTINATION ${DATA_INSTALL_DIR}/apps/color-schemes/)


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
