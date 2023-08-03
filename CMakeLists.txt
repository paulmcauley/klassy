cmake_minimum_required(VERSION 3.16)
project(klassy)
set(PROJECT_VERSION "5.99.breeze5.27.80")
set(PROJECT_VERSION_MAJOR 5)

add_definitions( -DKLASSY_VERSION="${PROJECT_VERSION}" -DKLASSY_GIT_MASTER=1)

set(KF_MIN_VERSION "5.102.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(QT_MIN_VERSION "5.15.2")

include(GenerateExportHeader)
include(WriteBasicConfigVersionFile)
include(FeatureSummary)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH}  ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMakePackageConfigHelpers)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(GtkUpdateIconCache)

option(WITH_DECORATIONS "Build Klassy window decorations for KWin" ON)

if(WIN32 OR APPLE OR QT_MAJOR_VERSION STREQUAL "5")
    set(WITH_DECORATIONS OFF)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets DBus Svg Xml) #TODO:Svg and Xml only used for klassy-settings
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Config
    GuiAddons
    I18n
    IconThemes
    WindowSystem)

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(KF6ColorScheme ${KF_MIN_VERSION} REQUIRED)
endif()

if (NOT WIN32 AND NOT APPLE)
    find_package(KF${QT_MAJOR_VERSION}KCMUtils ${KF_MIN_VERSION})
    set_package_properties(KF${QT_MAJOR_VERSION}KCMUtils PROPERTIES
        TYPE REQUIRED
        DESCRIPTION "Helps create configuration modules"
        PURPOSE "KCMUtils used for the configuration modules or the decoration and Qt Style"
    )
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Quick)
set(BREEZE_HAVE_QTQUICK ${Qt${QT_MAJOR_VERSION}Quick_FOUND})
if(BREEZE_HAVE_QTQUICK)
    find_package(KF${QT_MAJOR_VERSION} ${KF${QT_MAJOR_VERSION}_MIN_VERSION} REQUIRED COMPONENTS Kirigami2)
endif()

if(${QT_MAJOR_VERSION} EQUAL 5)
    find_package(Qt5 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS X11Extras)
    set(BREEZE_HAVE_QTX11EXTRAS ${Qt5X11Extras_FOUND})

    find_package(KF5ConfigWidgets ${KF_MIN_VERSION} REQUIRED)
elseif(NOT WIN32)
    set(BREEZE_HAVE_QTX11EXTRAS TRUE) # part of Qt6::Gui
endif()

find_package(KF${QT_MAJOR_VERSION}FrameworkIntegration ${KF${QT_MAJOR_VERSION}_MIN_VERSION} CONFIG)
set_package_properties(KF${QT_MAJOR_VERSION}FrameworkIntegration PROPERTIES
    DESCRIPTION "KF${QT_MAJOR_VERSION} Framework Integration"
    URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
    TYPE OPTIONAL
    PURPOSE "Required to use KStyle convenience functionalities in style")
set(BREEZE_HAVE_KSTYLE ${KF${QT_MAJOR_VERSION}FrameworkIntegration_FOUND})


find_package(KDecoration2 REQUIRED)
if(WITH_DECORATIONS)
    find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS DBus)
    add_subdirectory(kdecoration)
    add_subdirectory(icons)
    add_subdirectory(layout-templates)
    add_subdirectory(look-and-feel)
endif()

add_subdirectory(libbreezecommon)
add_subdirectory(kstyle)
add_subdirectory(colors)

ki18n_install(po)

include(ECMSetupVersion)
ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX BREEZE
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KlassyConfigVersion.cmake"
                  )

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/Klassy")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KlassyConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/KlassyConfig.cmake"
                                  PATH_VARS KDE_INSTALL_FULL_DATADIR
                                  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                                  )

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KlassyConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KlassyConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
        )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
