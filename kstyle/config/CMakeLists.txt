### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"klassy_style_config\")
set(kstyle_breeze_config_PART_SRCS breezestyleconfig.cpp ui/klassystyleconfig.qrc)

set(kstyle_breeze_config_PART_SRCS ${kstyle_breeze_config_PART_SRCS} breezestyleconfigmodule.cpp )

kconfig_add_kcfg_files(kstyle_breeze_config_PART_SRCS ../breezestyleconfigdata.kcfgc)
set(kstyle_breeze_config_PART_FORMS ui/breezestyleconfig.ui)
ki18n_wrap_ui(kstyle_breeze_config_PART_FORMS_HEADERS ${kstyle_breeze_config_PART_FORMS})

set(CMAKE_AUTORCC ON)

kcoreaddons_add_plugin(klassystyleconfig SOURCES
    ${kstyle_breeze_config_PART_SRCS}
    ${kstyle_breeze_config_PART_FORMS_HEADERS}
    INSTALL_NAMESPACE "kstyle_config"
)

target_link_libraries(klassystyleconfig Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus)
target_link_libraries(klassystyleconfig KF6::CoreAddons KF6::ConfigCore KF6::ConfigWidgets KF6::I18n)
if(QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(klassystyleconfig KF6::KCMUtils)
endif()
kcmutils_generate_desktop_file(klassystyleconfig)

include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon6)


set(breeze_settings_SOURCES
    main.cpp
    systemicongenerator.cpp
)
add_executable(klassy-settings ${breeze_settings_SOURCES} )
target_link_libraries(klassy-settings Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg Qt6::Xml)
target_link_libraries(klassy-settings KF6::I18n KF6::KCMUtils KF6::CoreAddons)
target_link_libraries(klassy-settings klassycommon6)

install(TARGETS klassy-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# icon
ecm_install_icons(ICONS sc-apps-klassy-settings.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor )

# .desktop file
install(FILES klassy-settings.desktop DESTINATION ${KDE_INSTALL_APPDIR})
