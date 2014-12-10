find_package(KDecoration2 REQUIRED)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config)


set(breezedecoration_SRCS
    breezecolorsettings.cpp
    breezedeco.cpp
    breezebuttons.cpp
)

add_library(breezedecoration MODULE ${breezedecoration_SRCS})

target_link_libraries(breezedecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
    PRIVATE
        KDecoration2::KDecoration
        KF5::ConfigCore
        KF5::CoreAddons
)

install(TARGETS breezedecoration DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
