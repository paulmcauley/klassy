kcoreaddons_add_plugin(kcm_klassydecoration SOURCES kcm_breezedecoration.cpp INSTALL_NAMESPACE "plasma/kcms/klassy")
target_include_directories(kcm_klassydecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_klassydecoration klassycommon5 klassydecoration_STATIC KF6::I18n KF6::CoreAddons KF6::ConfigWidgets KF6::I18n
        KF6::IconThemes Qt::DBus Qt::Widgets)
kcmutils_generate_desktop_file(kcm_klassydecoration)

#install bundled presets
install(DIRECTORY presets  DESTINATION  ${KDE_INSTALL_QTPLUGINDIR}/plasma/kcms/klassy)
