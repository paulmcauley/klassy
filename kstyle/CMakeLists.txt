project(kstyle-breeze)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include(WriteBasicConfigVersionFile)
include(FeatureSummary)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

################# Qt5/KF5 #################
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS
    I18n
    Config
    GuiAddons
    WidgetsAddons
    Service
    Completion
    FrameworkIntegration
    WindowSystem)

################# X11 #################
find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
    URL "http://www.x.org"
    TYPE OPTIONAL
    PURPOSE "Required for building the X11 based workspace"
)

add_definitions( -DHAVE_X11=${X11_FOUND} )

if(X11_FOUND)
  find_package(XCB REQUIRED COMPONENTS XCB)
  set_package_properties(XCB PROPERTIES TYPE REQUIRED)

  find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(GenerateExportHeader)

################# includes #################
include_directories( animations )

########### next target ###############
set(breeze_PART_SRCS
    animations/breezeanimation.cpp
    animations/breezeanimations.cpp
    animations/breezeanimationdata.cpp
    animations/breezebaseengine.cpp
    animations/breezebusyindicatorengine.cpp
    animations/breezedialdata.cpp
    animations/breezedialengine.cpp
    animations/breezeenabledata.cpp
    animations/breezegenericdata.cpp
    animations/breezeheaderviewdata.cpp
    animations/breezeheaderviewengine.cpp
    animations/breezescrollbardata.cpp
    animations/breezescrollbarengine.cpp
    animations/breezesliderengine.cpp
    animations/breezesliderdata.cpp
    animations/breezespinboxengine.cpp
    animations/breezespinboxdata.cpp
    animations/breezetabbarengine.cpp
    animations/breezetabbardata.cpp
    animations/breezewidgetstateengine.cpp
    animations/breezewidgetstatedata.cpp
    breezeframeshadow.cpp
    breezehelper.cpp
    breezemnemonics.cpp
    breezepropertynames.cpp
    breezeshadowhelper.cpp
    breezesplitterproxy.cpp
    breezestyle.cpp
    breezetileset.cpp
    breezewindowmanager.cpp
)

kconfig_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)

add_library(breeze MODULE ${breeze_PART_SRCS})

target_link_libraries(breeze  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

target_link_libraries(breeze Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
target_link_libraries(breeze KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::Style KF5::WindowSystem)

if(X11_FOUND)
  target_link_libraries(breeze ${XCB_LIBRARIES})
  target_link_libraries(breeze Qt5::X11Extras)
endif()

install(TARGETS breeze DESTINATION ${QT_PLUGIN_INSTALL_DIR}/styles/ )

########### install files ###############
install( FILES breeze.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )

########### summary ###############
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
