add_definitions(-DTRANSLATION_DOMAIN=\"breeze_decoration_config\")

find_package(Qt5 CONFIG REQUIRED COMPONENTS DBus )

set(breezedecoration_config_PART_SRCS
    breezeconfig.cpp
    breezeconfigwidget.cpp
 )

set(breezedecoration_config_PART_FORMS
   ui/breezeconfigurationui.ui
)

kconfig_add_kcfg_files(breezedecoration_config_PART_SRCS ../breezesettings.kcfgc)

ki18n_wrap_ui(breezedecoration_config_PART_FORMS_HEADERS ${breezedecoration_config_PART_FORMS})
add_library(breezedecoration_config MODULE
    ${breezedecoration_config_PART_SRCS}
    ${breezedecoration_config_PART_FORMS_HEADERS})

target_link_libraries(breezedecoration_config Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
target_link_libraries(breezedecoration_config KF5::I18n KF5::ConfigCore KF5::ConfigWidgets)

install(TARGETS breezedecoration_config DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
