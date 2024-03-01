################# breezestyle target #################
set(breezecommon_LIB_SRCS
    breeze.cpp
    breezeboxshadowrenderer.cpp
    colortools.cpp
    dbusupdatenotifier.cpp
    decorationbuttoncolors.cpp
    decorationcolors.cpp
    decorationexceptionlist.cpp
    geometrytools.cpp
    presetsmodel.cpp
    renderdecorationbuttonicon.cpp
    renderdecorationbuttonicon18by18.cpp
    styleklassy.cpp
    stylekite.cpp
    styleoxygen.cpp
    styleredmond.cpp
    styleredmond10.cpp
    styleredmond11.cpp
    systemicontheme.cpp
    dbusmessages.h
    setqdebug_logging.h
)

kconfig_add_kcfg_files(breezecommon_LIB_SRCS ../kdecoration/breezesettings.kcfgc)

add_library(klassycommon ${breezecommon_LIB_SRCS})

generate_export_header(klassycommon
    BASE_NAME breezecommon
    EXPORT_FILE_NAME breezecommon_export.h)

target_link_libraries(klassycommon
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus

        KDecoration2::KDecoration
    PRIVATE
        KF${QT_MAJOR_VERSION}::GuiAddons
        KF${QT_MAJOR_VERSION}::ConfigCore
        KF${QT_MAJOR_VERSION}::ConfigGui
        KF${QT_MAJOR_VERSION}::ConfigWidgets
        KF${QT_MAJOR_VERSION}::IconThemes
        KF${QT_MAJOR_VERSION}::I18n)

set_target_properties(klassycommon PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME klassycommon${QT_MAJOR_VERSION})

install(TARGETS klassycommon ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
