################# Qt/KDE #################
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets DBus)
find_package(KF5 REQUIRED COMPONENTS
    I18n
    Config
    GuiAddons
    IconThemes
    ConfigWidgets
    WindowSystem)

find_package(Qt5 COMPONENTS Quick)
set(BREEZE_HAVE_QTQUICK ${Qt5Quick_FOUND})

find_package( KF5FrameworkIntegration CONFIG )
set_package_properties(KF5FrameworkIntegration PROPERTIES
    DESCRIPTION "KF5 Framework Integration"
    URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
    TYPE OPTIONAL
    PURPOSE "Required to use KStyle convenience functionalities in style")

set(BREEZE_HAVE_KSTYLE ${KF5FrameworkIntegration_FOUND})

find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
    DESCRIPTION "X protocol C-language Binding"
    URL "https://xcb.freedesktop.org"
    TYPE OPTIONAL
    PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

find_package(KF5Wayland CONFIG)
set(BREEZE_HAVE_KWAYLAND ${KF5Wayland_FOUND})

if(UNIX AND NOT APPLE)
    set(BREEZE_HAVE_X11 ${XCB_XCB_FOUND})
    if (XCB_XCB_FOUND)
        find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
    endif()
else()
    set(BREEZE_HAVE_X11 FALSE)
endif()


################# includes #################
include_directories(
  animations
  debug
)

include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)

################# configuration #################
configure_file(config-breeze.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breeze.h )

########### next target ###############
set(breeze_PART_SRCS
    animations/breezeanimation.cpp
    animations/breezeanimations.cpp
    animations/breezeanimationdata.cpp
    animations/breezebaseengine.cpp
    animations/breezebusyindicatordata.cpp
    animations/breezebusyindicatorengine.cpp
    animations/breezedialdata.cpp
    animations/breezedialengine.cpp
    animations/breezeenabledata.cpp
    animations/breezegenericdata.cpp
    animations/breezeheaderviewdata.cpp
    animations/breezeheaderviewengine.cpp
    animations/breezescrollbardata.cpp
    animations/breezescrollbarengine.cpp
    animations/breezespinboxengine.cpp
    animations/breezespinboxdata.cpp
    animations/breezestackedwidgetdata.cpp
    animations/breezestackedwidgetengine.cpp
    animations/breezetabbarengine.cpp
    animations/breezetabbardata.cpp
    animations/breezetoolboxengine.cpp
    animations/breezetransitiondata.cpp
    animations/breezetransitionwidget.cpp
    animations/breezewidgetstateengine.cpp
    animations/breezewidgetstatedata.cpp
    debug/breezewidgetexplorer.cpp
    breezeaddeventfilter.cpp
    breezeblurhelper.cpp
    breezeframeshadow.cpp
    breezehelper.cpp
    breezemdiwindowshadow.cpp
    breezemnemonics.cpp
    breezepropertynames.cpp
    breezeshadowhelper.cpp
    breezesplitterproxy.cpp
    breezestyle.cpp
    breezestyleplugin.cpp
    breezetileset.cpp
    breezewindowmanager.cpp
)

kconfig_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)
add_library(breeze MODULE ${breeze_PART_SRCS})
target_link_libraries(breeze Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
if( BREEZE_HAVE_QTQUICK )
    target_link_libraries(breeze Qt5::Quick)
endif()
target_link_libraries(breeze KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::IconThemes KF5::WindowSystem)
target_link_libraries(breeze breezecommon5)

if(KF5FrameworkIntegration_FOUND)
    target_link_libraries(breeze KF5::Style)
endif()

if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(breeze PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()

if(BREEZE_HAVE_X11)
    target_link_libraries(breeze ${XCB_LIBRARIES})
    target_link_libraries(breeze Qt5::X11Extras)
endif()

if(BREEZE_HAVE_KWAYLAND)
    target_link_libraries(breeze KF5::WaylandClient)
endif()


########### install files ###############
install(TARGETS breeze DESTINATION ${QT_PLUGIN_INSTALL_DIR}/styles/)
install(FILES breeze.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes)

########### subdirectories ###############
add_subdirectory(config)
