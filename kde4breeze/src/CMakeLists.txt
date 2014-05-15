# In this CMakeLists.txt we define which files
# are used to compile the application
set(kde4breeze_SRCS
   application.cpp
   main.cpp
 )

# Set the name of the application
kde4_add_executable(kde4breeze ${kde4breeze_SRCS})

# Select which libraries we need to link to
target_link_libraries(kde4breeze ${KDE4_KDEUI_LIBS})

# Tell cmake to install the application binary
install(TARGETS kde4breeze ${INSTALL_TARGETS_DEFAULT_ARGS})

# Install the color scheme

