#ifndef BREEZE_STYLEREDMOND_H
#define BREEZE_STYLEREDMOND_H

/*
 * Copyright 2017  Paul McAuley <kde@paulmcauley.com>
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

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{
    
    class RenderStyleRedmond18By18 : public RenderDecorationButtonIcon18By18
    {
        public:
            /**
             * @brief Constructor - calls constructor of base class
             * 
             * @param painter A QPainter object already initialised with an 18x18 reference window.
             * @param pen QPen with width and color already initialised.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller 
             */
            RenderStyleRedmond18By18(QPainter* painter, const QPen& pen, const bool notInTitlebar) : RenderDecorationButtonIcon18By18(painter, pen, notInTitlebar){};
            
            void renderCloseIcon() override;
            void renderMaximizeIcon() override;
            void renderRestoreIcon() override;
            void renderMinimizeIcon() override;
            void renderKeepBehindIcon() override;
            void renderKeepInFrontIcon() override;
    };
    
}

#endif
