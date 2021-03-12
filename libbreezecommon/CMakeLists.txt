################# dependencies #################
### Qt/KDE
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)

################# breezestyle target #################
set(breezecommon_LIB_SRCS
    breezeboxshadowrenderer.cpp
    renderdecorationbuttonicon.cpp
    styleclassik.cpp
    stylekite.cpp
    styleoxygen.cpp
    styleredmond.cpp
)

add_library(classikstylescommon5 ${breezecommon_LIB_SRCS})

# PAM: check BASE_NAME 
generate_export_header(classikstylescommon5
    BASE_NAME breezecommon
    EXPORT_FILE_NAME breezecommon_export.h)

target_link_libraries(classikstylescommon5
    PUBLIC
        Qt::Core
        Qt::Gui)

set_target_properties(classikstylescommon5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS classikstylescommon5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
