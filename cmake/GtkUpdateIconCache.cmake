# SPDX-FileCopyrightText: 2015 Volker Krause <vkrause@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause


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
