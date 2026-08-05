add_definitions(-DTRANSLATION_DOMAIN="klassy_kwin_deco")

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon6)

################# newt target #################
add_subdirectory(appmenu)
add_subdirectory(libdbusmenuqt)

### plugin classes
set(breezedecoration_SRCS
    breezebutton.cpp
    breezedecoration.cpp
    breezesettingsprovider.cpp
    ${breezedecoration_appmenu_SRCS}
)

### build library
#This is removed as kdecoration InternalSettings is now generated from libbreezecommon
#kconfig_add_kcfg_files(klassydecoration_STATIC breezesettings.kcfgc)
add_library(klassydecoration MODULE
    ${breezedecoration_SRCS}
    ${breezedecoration_config_PART_FORMS_HEADERS})

set_target_properties(klassydecoration PROPERTIES
    OUTPUT_NAME org.kde.klassy
)

target_link_libraries(klassydecoration
    PRIVATE
        klassycommon6
        dbusmenuqt
        Qt6::DBus
        KF6::CoreAddons
        KF6::ConfigGui
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::WindowSystem
        KDecoration3::KDecoration
)

install(TARGETS klassydecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

add_subdirectory(config)
