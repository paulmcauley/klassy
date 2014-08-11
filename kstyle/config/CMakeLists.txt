add_definitions(-DTRANSLATION_DOMAIN=\"breeze_style_config\")
set(kstyle_breeze_config_PART_SRCS breezestyleconfig.cpp )

kconfig_add_kcfg_files(kstyle_breeze_config_PART_SRCS ../breezestyleconfigdata.kcfgc)

set( kstyle_breeze_config_PART_FORMS ui/breezestyleconfig.ui )

ki18n_wrap_ui(kstyle_breeze_config_PART_FORMS_HEADERS ${kstyle_breeze_config_PART_FORMS})

add_library(kstyle_breeze_config MODULE
    ${kstyle_breeze_config_PART_SRCS}
    ${kstyle_breeze_config_PART_FORMS_HEADERS}
)

target_link_libraries(kstyle_breeze_config Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
target_link_libraries(kstyle_breeze_config KF5::ConfigCore KF5::ConfigWidgets KF5::Completion KF5::I18n)

install(TARGETS kstyle_breeze_config DESTINATION ${QT_PLUGIN_INSTALL_DIR} )
