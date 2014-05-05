/********************************************************************
Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.0
import org.kde.kwin.decoration 0.1

DecorationButton {
    id: button
    property real size
    width: size
    height: size
    Rectangle {
        anchors {
            fill: parent
            margins: 3
        }
        radius: width/2.0
        color: options.fontColor
        opacity: 0.1
    }
   Component.onCompleted: {
       if (buttonType == DecorationOptions.DecorationButtonQuickHelp) {
           visible = decoration.providesContextHelp;
       }
       if (buttonType == DecorationOptions.DecorationButtonApplicationMenu) {
           visible = decoration.appMenu;
       }
    }
    Connections {
        target: decoration
        onAppMenuChanged: {
            if (buttonType == DecorationOptions.DecorationButtonApplicationMenu) {
                visible = decoration.appMenu;
            }
        }
    }
}
