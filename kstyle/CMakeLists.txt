project(kstyle-breeze)

include(FeatureSummary)

if( USE_KDE4 )

  ############ Language and toolchain features
  ############ copied from ECM
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND NOT WIN32)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif()

  add_definitions( -DUSE_KDE4=1 )

else()

  add_definitions( -DUSE_KDE4=0 )

endif()

################# X11 #################
find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
    URL "http://www.x.org"
    TYPE OPTIONAL
    PURPOSE "Required for building the X11 based workspace"
)

add_definitions( -DHAVE_X11=${X11_FOUND} )

################# Qt/KDE #################
if( USE_KDE4 )

  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)
  include(MacroLibrary)

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
  include_directories(${KDE4_INCLUDES})

  if( X11_FOUND )
    ### XCB
    find_package(PkgConfig REQUIRED)
    pkg_check_modules( XCB REQUIRED xcb x11-xcb)
  endif()

else()

  find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets DBus)
  find_package(KF5 REQUIRED COMPONENTS
      I18n
      Config
      Completion
      FrameworkIntegration
      WindowSystem)

  if(X11_FOUND)
    ### XCB
    find_package(XCB REQUIRED COMPONENTS XCB)
    find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
  endif()

endif()

################# includes #################
include_directories(
  animations
  debug
)

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
    animations/breezesliderengine.cpp
    animations/breezesliderdata.cpp
    animations/breezespinboxengine.cpp
    animations/breezespinboxdata.cpp
    animations/breezetabbarengine.cpp
    animations/breezetabbardata.cpp
    animations/breezetoolboxengine.cpp
    animations/breezewidgetstateengine.cpp
    animations/breezewidgetstatedata.cpp
    debug/breezewidgetexplorer.cpp
    breezeaddeventfilter.cpp
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
)

if( USE_KDE4 )

  kde4_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)
  kde4_add_plugin(breeze ${breeze_PART_SRCS})
  target_link_libraries(breeze  ${KDE4_KDEUI_LIBS} )

  if( X11_FOUND )
    target_link_libraries( breeze ${X11_X11_LIB} )
    target_link_libraries( breeze ${X11_XCB_LIBRARIES} )
    target_link_libraries( breeze ${XCB_LIBRARIES} )
  endif()

  install(TARGETS breeze  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles/ )

else()

  kconfig_add_kcfg_files(breeze_PART_SRCS breezestyleconfigdata.kcfgc)
  add_library(breeze MODULE ${breeze_PART_SRCS})
  target_link_libraries(breeze Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
  target_link_libraries(breeze KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::Style KF5::WindowSystem)

  if(X11_FOUND)
    target_link_libraries(breeze ${XCB_LIBRARIES})
    target_link_libraries(breeze Qt5::X11Extras)
  endif()

  install(TARGETS breeze DESTINATION ${QT_PLUGIN_INSTALL_DIR}/styles/ )

endif()

########### install files ###############
install( FILES breeze.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )

########### summary ###############
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

########### subdirectories ###############
add_subdirectory( config )
