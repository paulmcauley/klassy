add_definitions(-DTRANSLATION_DOMAIN="klassy_kwin_deco")

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS CoreAddons GuiAddons ConfigWidgets WindowSystem I18n IconThemes)
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS DBus Widgets)

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)

################# newt target #################
### plugin classes
set(breezedecoration_SRCS
    breezebutton.cpp
    breezedecoration.cpp
    breezesettingsprovider.cpp
)

### config classes
set(breezedecoration_config_SRCS
    config/breezeconfigwidget.cpp
    config/breezedetectwidget.cpp
    config/breezeexceptiondialog.cpp
    config/breezeexceptionlistwidget.cpp
    config/breezeexceptionmodel.cpp
    config/breezeitemmodel.cpp
    config/buttonsizing.cpp
    config/windowoutlinestyle.cpp
    config/loadpreset.cpp
    config/addpreset.cpp
    config/buttoncolors.cpp
    config/buttonbehaviour.cpp
    config/systemicongeneration.cpp
    config/titlebarspacing.cpp
    config/titlebaropacity.cpp
    config/shadowstyle.cpp
)

ki18n_wrap_ui(breezedecoration_config_SRCS
   config/ui/breezeconfigurationui.ui
   config/ui/breezeexceptiondialog.ui
   config/ui/breezeexceptionlistwidget.ui
   config/ui/buttonsizing.ui
   config/ui/windowoutlinestyle.ui
   config/ui/loadpreset.ui
   config/ui/addpreset.ui
   config/ui/buttoncolors.ui
   config/ui/buttonbehaviour.ui
   config/ui/systemicongeneration.ui
   config/ui/titlebarspacing.ui
   config/ui/titlebaropacity.ui
   config/ui/shadowstyle.ui
)

qt_add_resources(breezedecoration_config_SRCS config/ui/klassydecoration_config.qrc)

add_library(klassydecoration_STATIC STATIC ${breezedecoration_config_SRCS})
# Needed to link this static lib to shared libs
set_property(TARGET klassydecoration_STATIC PROPERTY POSITION_INDEPENDENT_CODE ON)

#This is removed as kdecoration InternalSettings is now generated from libbreezecommon
#kconfig_add_kcfg_files(klassydecoration_STATIC breezesettings.kcfgc)
target_link_libraries(klassydecoration_STATIC
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus
        KDecoration2::KDecoration
    PRIVATE
        #KF6::ConfigCore
        klassycommon
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::WindowSystem
        KF6::KCMUtils
)

### build library
add_library(klassydecoration MODULE
    ${breezedecoration_SRCS}
    ${breezedecoration_config_PART_FORMS_HEADERS})

set_target_properties(klassydecoration PROPERTIES
    OUTPUT_NAME org.kde.klassy
)

target_link_libraries(klassydecoration
    PRIVATE
        klassycommon
        klassydecoration_STATIC
        KF6::CoreAddons
        KF6::ConfigWidgets
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::WindowSystem
        KF6::KCMUtils)

install(TARGETS klassydecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2)

add_subdirectory(config)
