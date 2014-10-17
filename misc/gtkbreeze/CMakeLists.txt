project(gtkbreeze)

find_package(ECM 0.0.13 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMPackageConfigHelpers)
include(ECMMarkNonGuiExecutable)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(FeatureSummary)
include(KDEFrameworkCompilerSettings)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

# In this CMakeLists.txt we define which files
# are used to compile the application
set(gtkbreeze_SRCS
   main.cpp
 )

# Set the name of the application
add_executable(gtkbreeze ${gtkbreeze_SRCS})

# Select which libraries we need to link to
target_link_libraries(gtkbreeze Qt5::Widgets KF5::CoreAddons KF5::ConfigCore)

# Tell cmake to install the application binary
install(TARGETS gtkbreeze DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)
install(FILES gtkbreeze.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

# Install the color scheme?

