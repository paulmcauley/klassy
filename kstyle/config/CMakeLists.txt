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
    INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets"
)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Svg Xml)

target_link_libraries(klassystyleconfig Qt::Core Qt::Gui Qt::Widgets Qt::DBus)
target_link_libraries(klassystyleconfig KF${QT_MAJOR_VERSION}::CoreAddons KF${QT_MAJOR_VERSION}::ConfigCore KF${QT_MAJOR_VERSION}::ConfigWidgets KF${QT_MAJOR_VERSION}::I18n)
kcmutils_generate_desktop_file(klassystyleconfig)

include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)


set(breeze_settings_SOURCES
    main.cpp
    systemicongenerator.cpp
)
add_executable(klassy-settings ${breeze_settings_SOURCES} )
target_link_libraries(klassy-settings Qt::Core Qt::Gui Qt::Widgets Qt::Svg Qt::Xml)
target_link_libraries(klassy-settings KF${QT_MAJOR_VERSION}::I18n KF${QT_MAJOR_VERSION}::KCMUtils KF${QT_MAJOR_VERSION}::CoreAddons)
target_link_libraries(klassy-settings klassycommon5)

install(TARGETS klassy-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# icon
ecm_install_icons(ICONS sc-apps-klassy-settings.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor )

# .desktop file
install(FILES klassy-settings.desktop DESTINATION ${KDE_INSTALL_APPDIR})
