################# breezestyle target #################
set(breezecommon_LIB_SRCS
    breeze.cpp
    breezeboxshadowrenderer.cpp
    colortools.cpp
    decorationbuttoncolors.cpp
    decorationcolors.cpp
    decorationexceptionlist.cpp
    geometrytools.cpp
    presetsmodel.cpp
    renderdecorationbuttonicon.cpp
    renderdecorationbuttonicon18by18.cpp
    stylekisweet.cpp
    styleklasse.cpp
    stylekite.cpp
    styleoxygen.cpp
    styletraditional.cpp
    stylemetro.cpp
    stylefluent.cpp
    systemicontheme.cpp
    setqdebug_logging.h
)

if(HAVE_QTDBUS)
    list(APPEND breezecommon_LIB_SRCS
        dbusupdatenotifier.cpp
        dbusmessages.h
    )
endif()

kconfig_add_kcfg_files(breezecommon_LIB_SRCS breezesettings.kcfgc)

add_library(klassycommon${QT_MAJOR_VERSION} ${breezecommon_LIB_SRCS})

generate_export_header(klassycommon${QT_MAJOR_VERSION}
    BASE_NAME breezecommon
    EXPORT_FILE_NAME breezecommon_export.h)

target_link_libraries(klassycommon${QT_MAJOR_VERSION}
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Widgets

        KF${QT_MAJOR_VERSION}::GuiAddons
        KF${QT_MAJOR_VERSION}::ConfigCore
        KF${QT_MAJOR_VERSION}::ConfigGui
        KF${QT_MAJOR_VERSION}::IconThemes
        KF${QT_MAJOR_VERSION}::I18n)

if(HAVE_QTDBUS)
    target_link_libraries(klassycommon${QT_MAJOR_VERSION}
    PUBLIC
        Qt${QT_MAJOR_VERSION}::DBus
    )
endif()

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(klassycommon${QT_MAJOR_VERSION} PUBLIC KF5::ConfigWidgets)
else()
    target_link_libraries(klassycommon${QT_MAJOR_VERSION} PUBLIC KF6::ColorScheme)
endif()

set_target_properties(klassycommon${QT_MAJOR_VERSION} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME klassycommon${QT_MAJOR_VERSION})

install(TARGETS klassycommon${QT_MAJOR_VERSION} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
