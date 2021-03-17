################# dependencies #################
### Qt/KDE
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS Config)

################# breezestyle target #################
set(breezecommon_LIB_SRCS
    breezeboxshadowrenderer.cpp
    renderdecorationbuttonicon.cpp
    styleclassik.cpp
    stylekite.cpp
    styleoxygen.cpp
    styleredmond.cpp
)

kconfig_add_kcfg_files(breezecommon_LIB_SRCS ../kdecoration/breezesettings.kcfgc)

add_library(classikstylescommon5 ${breezecommon_LIB_SRCS})

generate_export_header(classikstylescommon5
    BASE_NAME breezecommon
    EXPORT_FILE_NAME breezecommon_export.h)

target_link_libraries(classikstylescommon5
    PUBLIC
        Qt::Core
        Qt::Gui
        
        KF5::ConfigCore
        KF5::ConfigGui)

set_target_properties(classikstylescommon5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS classikstylescommon5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
