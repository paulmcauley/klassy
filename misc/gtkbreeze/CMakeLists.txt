project(gtkbreeze)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config)

set(gtkbreeze_SRCS
   main.cpp
 )

add_executable(gtkbreeze ${gtkbreeze_SRCS})

target_link_libraries(gtkbreeze Qt5::Widgets KF5::CoreAddons KF5::ConfigCore)

install(TARGETS gtkbreeze DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)
install(FILES gtkbreeze.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
