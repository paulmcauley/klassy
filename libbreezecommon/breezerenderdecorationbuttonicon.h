#ifndef BREEZE_RENDERDECORATIONBUTTONICON_H
#define BREEZE_RENDERDECORATIONBUTTONICON_H

/*
 * Copyright 2020  Paul McAuley <kde@paulmcauley.com>
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

#include "breezecommon_export.h"

#include <QPainter>
#include <QPainterPath>
#include <memory>

namespace Breeze
{
    
    /**
     * @brief Base Class to render decoration button icons in style set by EnumButtonIconStyle.
     *        Rendering is to be performed on a QPainter object with an 18x18 reference window.
     *        Co-ordinates relative to top-left.
     *        To be used as common code base across both kdecoration and kstyle.
     */
    class BREEZECOMMON_EXPORT RenderDecorationButtonIcon18By18
    {
    
        public:
            
            /**
             * @brief Button Icon styles as defined in the .kcfg files.
             */
            enum EnumButtonIconStyle { StyleClassic, StyleOxygen, StyleRedmond };
            

            /**
             * @brief Factory to return a pointer to a new inherited object to render in the specified style.
             * 
             * @param painter A QPainter object already initialised with an 18x18 reference window.
             * @param pen QPen with width and color already initialized.
             * @param buttonIconStyle The desired icon style as equivalent to type EnumButtonIconStyle.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
             * @return std::unique_ptr< Breeze::RenderDecorationButtonIcon18By18, std::default_delete< Breeze::RenderDecorationButtonIcon18By18 > > Pointer to a new sub-style object.
             */
            static std::unique_ptr<RenderDecorationButtonIcon18By18> factory( QPainter* painter, const QPen& pen, int buttonIconStyle = int(StyleOxygen), const bool notInTitlebar = false);
            
            virtual ~RenderDecorationButtonIcon18By18(){};
            
            virtual void renderCloseIcon();
            virtual void renderMaximizeIcon();
            virtual void renderRestoreIcon();
            virtual void renderMinimizeIcon();
            virtual void renderPinnedOnAllDesktopsIcon();
            virtual void renderPinOnAllDesktopsIcon();
            virtual void renderShadeIcon();
            virtual void renderUnShadeIcon();
            virtual void renderKeepBehindIcon();
            virtual void renderKeepInFrontIcon();
            virtual void renderApplicationMenuIcon();
            virtual void renderContextHelpIcon();
            
        protected:
            
            /**
             * @brief Constructor
             * 
             * @param painter A QPainter object already initialised with an 18x18 reference window.
             * @param pen QPen with width and color already initialised.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
             */
            RenderDecorationButtonIcon18By18( QPainter* painter, const QPen& pen, const bool notInTitlebar);
            
            
            /**
             * @brief Initialises pen to standardise cap and join styles. 
             * No brush is normal for Breeze's simple outline style.
             */
            virtual void initPainter();
            
            QPainter* painter;
            QPen pen;
            bool notInTitlebar;
            
    };
    
}

#endif
