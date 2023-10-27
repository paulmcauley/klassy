################# includes #################
include_directories(
  animations
  debug
)

include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon${QT_MAJOR_VERSION})

################# configuration #################
configure_file(config-breeze.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breeze.h )

########### next target ###############
set(breeze_PART_SRCS
    animations/breezeanimation.cpp
    animations/breezeanimations.cpp
    animations/breezeanimationdata.cpp
    animations/breezebaseengine.cpp
    animations/breezebusyindicatordata.cpp
    animations/breezebusyindicatorengine.cpp
    animations/breezedialdata.cpp
    animations/breezedialengine.cpp
    animations/breezeenabledata.cpp
    animations/breezegenericdata.cpp
    animations/breezeheaderviewdata.cpp
    animations/breezeheaderviewengine.cpp
    animations/breezescrollbardata.cpp
    animations/breezescrollbarengine.cpp
    animations/breezespinboxengine.cpp
    animations/breezespinboxdata.cpp
    animations/breezestackedwidgetdata.cpp
    animations/breezestackedwidgetengine.cpp
    animations/breezetabbarengine.cpp
    animations/breezetabbardata.cpp
    animations/breezetoolboxengine.cpp
    animations/breezetransitiondata.cpp
    animations/breezetransitionwidget.cpp
    animations/breezewidgetstateengine.cpp
    animations/breezewidgetstatedata.cpp
    debug/breezewidgetexplorer.cpp
    breezeaddeventfilter.cpp
    breezeblurhelper.cpp
    breezedecorationsettingsprovider.cpp
    breezeframeshadow.cpp
    breezehelper.cpp
    breezemdiwindowshadow.cpp
    breezemnemonics.cpp
    breezepropertynames.cpp
    breezeshadowhelper.cpp
    breezesplitterproxy.cpp
    breezestyle.cpp
    breezestyleplugin.cpp
    breezetileset.cpp
    breezewindowmanager.cpp
    breezetoolsareamanager.cpp
)

#removed as the kdecoration kfgc is now processed in libbreezecommon
#kconfig_add_kcfg_files(breeze_PART_SRCS ../kdecoration/breezesettings.kcfgc)
kconfig_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)

add_library(klassy${QT_MAJOR_VERSION} MODULE ${breeze_PART_SRCS})

target_link_libraries(klassy${QT_MAJOR_VERSION}
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::DBus
)

if( BREEZE_HAVE_QTQUICK )
    target_link_libraries(klassy${QT_MAJOR_VERSION}
        Qt${QT_MAJOR_VERSION}::Quick
        KF${QT_MAJOR_VERSION}::CoreAddons
    )
endif()

target_link_libraries(klassy${QT_MAJOR_VERSION}
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::GuiAddons
    KF${QT_MAJOR_VERSION}::IconThemes
    KF${QT_MAJOR_VERSION}::WindowSystem
)

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(klassy5 KF5::ConfigWidgets)
    if (BREEZE_HAVE_QTQUICK)
        target_link_libraries(klassy5 KF5::Kirigami2)
    endif()
else()
    target_link_libraries(klassy6 KF6::ColorScheme)
    if (BREEZE_HAVE_QTQUICK)
        target_link_libraries(klassy6 KF6::KirigamiPlatform)
    endif()
endif()


target_link_libraries(klassy${QT_MAJOR_VERSION} klassycommon${QT_MAJOR_VERSION})

if(KF${QT_MAJOR_VERSION}FrameworkIntegration_FOUND)
    target_link_libraries(klassy${QT_MAJOR_VERSION} KF${QT_MAJOR_VERSION}::Style)
endif()

if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(klassy${QT_MAJOR_VERSION} PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()


########### install files ###############
install(TARGETS klassy${QT_MAJOR_VERSION} DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles/)
install(FILES klassy.themerc  DESTINATION  ${KDE_INSTALL_DATADIR}/kstyle/themes)

########### subdirectories ###############

if (QT_MAJOR_VERSION EQUAL "6" AND TARGET "KF6::KCMUtils")
    add_subdirectory(config)
endif()
