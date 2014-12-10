#ifndef breeze_h
#define breeze_h

/*
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

namespace Breeze
{

    //* metrics
    enum Metrics
    {

        //* corner radius
        Frame_FrameRadius = 3,

        //* top title bar edge
        TitleBar_TopMargin = 4,
        TitleBar_BottomMargin = 5,
        TitleBar_SideMargin = 5,
        TitleBar_ButtonSpacing = 3,

        // shadow dimensions
        Shadow_Size = 20,
        Shadow_Offset = 6,
        Shadow_Overlap = 2

    };

    //* button state
    //* todo: should we used bits ?
    enum class ButtonState : uint
    {
        Normal,
        Disabled,
        Hovered,
        Pressed,
        Checked,
        CheckedHovered,
        CheckedPressed,
        Preview
    };

    inline uint qHash(const ButtonState &state)
    { return static_cast<uint>(state); }

}

#endif
