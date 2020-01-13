### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"breeze_style_config\")
set(kstyle_breeze_config_PART_SRCS breezestyleconfig.cpp)

set(kstyle_breeze_config_PART_SRCS ${kstyle_breeze_config_PART_SRCS} breezestyleconfigmodule.cpp)

kconfig_add_kcfg_files(kstyle_breeze_config_PART_SRCS ../breezestyleconfigdata.kcfgc)
set(kstyle_breeze_config_PART_FORMS ui/breezestyleconfig.ui)
ki18n_wrap_ui(kstyle_breeze_config_PART_FORMS_HEADERS ${kstyle_breeze_config_PART_FORMS})

add_library(kstyle_breeze_config MODULE
    ${kstyle_breeze_config_PART_SRCS}
    ${kstyle_breeze_config_PART_FORMS_HEADERS}
)

target_link_libraries(kstyle_breeze_config Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
target_link_libraries(kstyle_breeze_config KF5::ConfigCore KF5::ConfigWidgets KF5::I18n)
install(TARGETS kstyle_breeze_config DESTINATION ${QT_PLUGIN_INSTALL_DIR})
install(FILES breezestyleconfig.desktop DESTINATION  ${SERVICES_INSTALL_DIR})

########### next target ###############
find_package(KF5KCMUtils CONFIG)
set_package_properties(KF5KCMUtils PROPERTIES
    DESCRIPTION "KF5 KCM utils Framework"
    URL "https://projects.kde.org/projects/frameworks/kcmutils"
    TYPE OPTIONAL
    PURPOSE "Required for breeze-settings5 application to build")

if( KF5KCMUtils_FOUND )
    set(breeze_settings_SOURCES main.cpp)
    add_executable(breeze-settings5 ${breeze_settings_SOURCES} )
    target_link_libraries(breeze-settings5 Qt5::Core Qt5::Gui Qt5::Widgets )
    target_link_libraries(breeze-settings5 KF5::I18n KF5::KCMUtils)

    install(TARGETS breeze-settings5 ${INSTALL_TARGETS_DEFAULT_ARGS})

    # icon
    ecm_install_icons(ICONS sc-apps-breeze-settings.svgz
        DESTINATION ${ICON_INSTALL_DIR}
        THEME hicolor )
endif()
