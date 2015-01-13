### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"breeze_style_config\")
set(kstyle_breeze_config_PART_SRCS breezestyleconfig.cpp)

if(BREEZE_USE_KDE4)

  kde4_add_kcfg_files(kstyle_breeze_config_PART_SRCS ../breezestyleconfigdata.kcfgc)
  kde4_add_ui_files(kstyle_breeze_config_PART_SRCS ui/breezestyleconfig.ui)
  kde4_add_plugin(kstyle_breeze_config ${kstyle_breeze_config_PART_SRCS})

  target_link_libraries(kstyle_breeze_config ${KDE4_KDEUI_LIBS} )
  install(TARGETS kstyle_breeze_config DESTINATION ${PLUGIN_INSTALL_DIR})

else()

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

endif()

########### next target ###############
if(KF5Service_FOUND AND NOT BREEZE_USE_KDE4)

  set(breeze_settings_SOURCES
    breezeconfigdialog.cpp
    main.cpp)

  set(breeze_settings_FORMS ui/breezeconfigdialog.ui)

  ki18n_wrap_ui(breeze_settings_FORMS_HEADERS ${breeze_settings_FORMS})

  add_executable(breeze-settings5
      ${breeze_settings_SOURCES}
      ${breeze_settings_FORMS_HEADERS}
 )

  target_link_libraries(breeze-settings5 Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
  target_link_libraries(breeze-settings5 KF5::I18n KF5::WidgetsAddons KF5::ConfigCore KF5::ConfigGui KF5::Service)

  install(TARGETS breeze-settings5 ${INSTALL_TARGETS_DEFAULT_ARGS})

  # icon
  message( "ICON_INSTALL_DIR: ${ICON_INSTALL_DIR}" )
  ecm_install_icons(ICONS sc-apps-breeze-settings.svgz
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor )

endif()
