#ifndef BREEZE_STYLEOXYGEN_H
#define BREEZE_STYLEOXYGEN_H

/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{
    
    class RenderStyleOxygen18By18 : public RenderDecorationButtonIcon18By18
    {
        public:
            /**
             * @brief Constructor - calls constructor of base class
             * 
             * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
             * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
             */
            RenderStyleOxygen18By18( QPainter* painter, const bool notInTitlebar, const bool boldButtonIcons ) 
                : RenderDecorationButtonIcon18By18( painter, notInTitlebar, boldButtonIcons ){};
            
            void renderCloseIcon() override;
            void renderMaximizeIcon() override;
            void renderRestoreIcon() override;
            void renderMinimizeIcon() override;
            void renderKeepBehindIcon() override;
            void renderKeepInFrontIcon() override;
            void renderContextHelpIcon() override;
            void renderShadeIcon() override;
            void renderUnShadeIcon() override;
    };
    
}

#endif
