# Copyright 2015 Volker Krause <vkrause@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file COPYING-CMAKE-SCRIPTS for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.

find_program(GTK_UPDATE_ICON_CACHE_EXECUTABLE NAMES gtk-update-icon-cache)

macro(gtk_update_icon_cache _dir)
    if (GTK_UPDATE_ICON_CACHE_EXECUTABLE)
        install(CODE "
            set(DESTDIR_VALUE \"\$ENV{DESTDIR}\")
            if (NOT DESTDIR_VALUE)
                execute_process(COMMAND ${GTK_UPDATE_ICON_CACHE_EXECUTABLE} -q -i . WORKING_DIRECTORY ${_dir})
            endif()
        ")
    endif()
endmacro()
