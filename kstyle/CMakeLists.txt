################# Qt/KDE #################
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets DBus)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Config
    GuiAddons
    IconThemes
    ConfigWidgets
    WindowSystem)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} OPTIONAL_COMPONENTS Quick)
set(BREEZE_HAVE_QTQUICK ${Qt${QT_MAJOR_VERSION}Quick_FOUND})
if( BREEZE_HAVE_QTQUICK )
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2)
endif()

if (TARGET Qt5::Core)
    find_package(Qt5 ${QT_MIN_VERSION} OPTIONAL_COMPONENTS X11Extras)
    set(BREEZE_HAVE_QTX11EXTRAS ${Qt5X11Extras_FOUND})
else()
    set(BREEZE_HAVE_QTX11EXTRAS TRUE) # part of Qt6::Gui
endif()

find_package( KF5FrameworkIntegration ${KF5_MIN_VERSION} CONFIG )
set_package_properties(KF5FrameworkIntegration PROPERTIES
    DESCRIPTION "KF5 Framework Integration"
    URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
    TYPE OPTIONAL
    PURPOSE "Required to use KStyle convenience functionalities in style")

set(BREEZE_HAVE_KSTYLE ${KF5FrameworkIntegration_FOUND})


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
    breezedecorationsettingsprovider.cpp
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
    breezetoolsareamanager.cpp
)

#removed as the kdecoration kfgc is now processed in libbreezecommon
#kconfig_add_kcfg_files(breeze_PART_SRCS ../kdecoration/breezesettings.kcfgc)
kconfig_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)
add_library(klassy MODULE ${breeze_PART_SRCS})
target_link_libraries(klassy Qt::Core Qt::Gui Qt::Widgets Qt::DBus)
if( BREEZE_HAVE_QTQUICK )
    target_link_libraries(klassy Qt::Quick KF5::CoreAddons KF5::Kirigami2)
endif()
if (TARGET Qt5::X11Extras)
    target_link_libraries(klassy Qt5::X11Extras)
elseif (TARGET Qt6::Gui)
    target_link_libraries(klassy Qt6::GuiPrivate)
endif()
target_link_libraries(klassy KF5::CoreAddons KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::IconThemes KF5::WindowSystem)
target_link_libraries(klassy klassycommon5)

if(KF5FrameworkIntegration_FOUND)
    target_link_libraries(klassy KF5::Style)
endif()

if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(klassy PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()


########### install files ###############
install(TARGETS klassy DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles/)
install(FILES klassy.themerc  DESTINATION  ${KDE_INSTALL_DATADIR}/kstyle/themes)

########### subdirectories ###############
add_subdirectory(config)
