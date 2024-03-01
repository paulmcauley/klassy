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

kcoreaddons_add_plugin(kcm_klassydecoration SOURCES kcm_breezedecoration.cpp ${breezedecoration_config_SRCS} INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}")
#This is removed as kdecoration InternalSettings is now generated from libbreezecommon
#kconfig_add_kcfg_files(klassydecoration_STATIC breezesettings.kcfgc)
target_include_directories(kcm_klassydecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_klassydecoration
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus
        Qt::Widgets
        KDecoration2::KDecoration
    PRIVATE
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::WindowSystem
        KF6::KCMUtils
        klassycommon
)

kcmutils_generate_desktop_file(kcm_klassydecoration)

#install bundled presets
install(DIRECTORY presets  DESTINATION  ${KDE_INSTALL_QTPLUGINDIR}/plasma/kcms/klassy)
