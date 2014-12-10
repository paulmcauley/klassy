find_package(KDecoration2 REQUIRED)
find_package(KF5 REQUIRED COMPONENTS CoreAddons ConfigWidgets)
find_package(Qt5 CONFIG REQUIRED COMPONENTS DBus )


set(breezedecoration_SRCS
    breezecolorsettings.cpp
    breezebutton.cpp
    breezedecoration.cpp
    config/breezeconfig.cpp
    config/breezeconfigwidget.cpp)

kconfig_add_kcfg_files(breezedecoration_SRCS breezesettings.kcfgc)

set(breezedecoration_PART_FORMS
   config/ui/breezeconfigurationui.ui
)

ki18n_wrap_ui(breezedecoration_PART_FORMS_HEADERS ${breezedecoration_PART_FORMS})

add_library(breezedecoration MODULE
    ${breezedecoration_SRCS}
    ${breezedecoration_PART_FORMS_HEADERS})

target_link_libraries(breezedecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::DBus
    PRIVATE
        KDecoration2::KDecoration
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::GuiAddons
        KF5::I18n
)

install(TARGETS breezedecoration DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
