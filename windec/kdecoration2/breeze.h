#ifndef breeze_h
#define breeze_h
/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

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

}

#endif
