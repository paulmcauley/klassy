### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"klassy_style_config\")
set(kstyle_breeze_config_PART_SRCS breezestyleconfig.cpp ui/klassystyleconfig.qrc)

set(kstyle_breeze_config_PART_SRCS ${kstyle_breeze_config_PART_SRCS} breezestyleconfigmodule.cpp )

kconfig_add_kcfg_files(kstyle_breeze_config_PART_SRCS ../breezestyleconfigdata.kcfgc)
set(kstyle_breeze_config_PART_FORMS ui/breezestyleconfig.ui)
ki18n_wrap_ui(kstyle_breeze_config_PART_FORMS_HEADERS ${kstyle_breeze_config_PART_FORMS})

set(CMAKE_AUTORCC ON)

add_library(kstyle_klassy_config MODULE
    ${kstyle_breeze_config_PART_SRCS}
    ${kstyle_breeze_config_PART_FORMS_HEADERS}
)

target_link_libraries(kstyle_klassy_config Qt::Core Qt::Gui Qt::Widgets Qt::DBus)
target_link_libraries(kstyle_klassy_config KF5::CoreAddons KF5::ConfigCore KF5::ConfigWidgets KF5::I18n)
install(TARGETS kstyle_klassy_config DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
install(FILES klassystyleconfig.desktop DESTINATION  ${KDE_INSTALL_KSERVICESDIR})

########### next target ###############
find_package(KF5KCMUtils ${KF5_MIN_VERSION} CONFIG)
set_package_properties(KF5KCMUtils PROPERTIES
    DESCRIPTION "KF5 KCM utils Framework"
    URL "https://projects.kde.org/projects/frameworks/kcmutils"
    TYPE OPTIONAL
    PURPOSE "Required for klassy-settings application to build")

if( KF5KCMUtils_FOUND )
    set(breeze_settings_SOURCES main.cpp)
    add_executable(klassy-settings ${breeze_settings_SOURCES} )
    target_link_libraries(klassy-settings Qt::Core Qt::Gui Qt::Widgets )
    target_link_libraries(klassy-settings KF5::I18n KF5::KCMUtils)

    install(TARGETS klassy-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

    # icon
    ecm_install_icons(ICONS sc-apps-klassy-settings.svgz
        DESTINATION ${KDE_INSTALL_ICONDIR}
        THEME hicolor )
endif()
