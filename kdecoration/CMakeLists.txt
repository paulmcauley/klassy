add_definitions(-DTRANSLATION_DOMAIN="klassy_kwin_deco")

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS CoreAddons GuiAddons ConfigWidgets WindowSystem I18n IconThemes)
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS DBus)

### XCB
find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
  DESCRIPTION "X protocol C-language Binding"
  URL "https://xcb.freedesktop.org"
  TYPE OPTIONAL
  PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

if(UNIX AND NOT APPLE)

  set(BREEZE_HAVE_X11 ${XCB_XCB_FOUND})
  if (XCB_XCB_FOUND AND TARGET Qt5::Core)
    find_package(Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS X11Extras)
  endif()

else()

  set(BREEZE_HAVE_X11 FALSE)

endif()

set(CMAKE_AUTORCC ON)

################# configuration #################
configure_file(config-breeze.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breeze.h )

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)

################# newt target #################
### plugin classes
set(breezedecoration_SRCS
    breezebutton.cpp
    breezedecoration.cpp
    breezesettingsprovider.cpp
    breezesizegrip.cpp)

# This is removed as kdecoration InternalSettings is now generated from libbreezecommon
#kconfig_add_kcfg_files(breezedecoration_SRCS breezesettings.kcfgc)

### config classes
### they are kept separately because they might move in a separate library in the future
set(breezedecoration_config_SRCS
    config/breezeconfigwidget.cpp
    config/breezedetectwidget.cpp
    config/breezeexceptiondialog.cpp
    config/breezeexceptionlistwidget.cpp
    config/breezeexceptionmodel.cpp
    config/breezeitemmodel.cpp
    config/buttonsizing.cpp
    config/ui/klassydecoration_config.qrc
)

set(breezedecoration_config_PART_FORMS
   config/ui/breezeconfigurationui.ui
   config/ui/breezedetectwidget.ui
   config/ui/breezeexceptiondialog.ui
   config/ui/breezeexceptionlistwidget.ui
   config/ui/buttonsizing.ui
)

ki18n_wrap_ui(breezedecoration_config_PART_FORMS_HEADERS ${breezedecoration_config_PART_FORMS})

### build library
add_library(klassydecoration MODULE
    ${breezedecoration_SRCS}
    ${breezedecoration_config_SRCS}
    ${breezedecoration_config_PART_FORMS_HEADERS})

target_link_libraries(klassydecoration
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus
    PRIVATE
        klassycommon5
        KDecoration2::KDecoration
        #KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::GuiAddons
        KF5::I18n
        KF5::IconThemes
        KF5::WindowSystem)

if(BREEZE_HAVE_X11)
  target_link_libraries(klassydecoration PUBLIC XCB::XCB)
  if (TARGET Qt5::X11Extras)
    target_link_libraries(klassydecoration PUBLIC Qt5::X11Extras)
  else ()
    target_link_libraries(klassydecoration PUBLIC Qt6::GuiPrivate)
  endif()
endif()


install(TARGETS klassydecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2)
install(FILES config/klassydecorationconfig.desktop DESTINATION  ${KDE_INSTALL_KSERVICESDIR})
