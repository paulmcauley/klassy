find_package(KDecoration2 REQUIRED)
find_package(KF5 REQUIRED COMPONENTS CoreAddons ConfigWidgets)


set(breezedecoration_SRCS
    breezecolorsettings.cpp
    breezebutton.cpp
    breezedecoration.cpp
    breezeimageprovider.cpp
)

kconfig_add_kcfg_files(breezedecoration_SRCS breezesettings.kcfgc)

add_library(breezedecoration MODULE ${breezedecoration_SRCS})

target_link_libraries(breezedecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
    PRIVATE
        KDecoration2::KDecoration
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
)

install(TARGETS breezedecoration DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)

add_subdirectory(config)
