#ifndef BREEZE_RENDERDECORATIONBUTTONICON_H
#define BREEZE_RENDERDECORATIONBUTTONICON_H

/*
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezecommon_export.h"
#include "breezesettings.h"

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
             * @brief Factory to return a pointer to a new inherited object to render in the specified style.
             * @param internalSettings An InternalSettingsPtr from the Window decoration config
             * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
             * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
             * @return std::unique_ptr< Breeze::RenderDecorationButtonIcon18By18, std::default_delete< Breeze::RenderDecorationButtonIcon18By18 > > Pointer to a new sub-style object.
             */
            static std::unique_ptr<RenderDecorationButtonIcon18By18> factory( const QSharedPointer<InternalSettings>& internalSettings, QPainter* painter, const bool notInTitlebar = false, const bool boldButtonIcons = false );
            
            virtual ~RenderDecorationButtonIcon18By18();
            
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
             * @param internalSettings An InternalSettingsPtr from the Window decoration config
             * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- usually means will be smaller
             * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
             */
            RenderDecorationButtonIcon18By18( QPainter* painter, const bool notInTitlebar, const bool boldButtonIcons );
            
            
            /**
             * @brief Initialises pen to standardise cap and join styles. 
             * No brush is normal for Breeze's simple outline style.
             */
            virtual void initPainter();
            
            QPainter* painter;
            QPen pen;
            bool notInTitlebar;
            bool boldButtonIcons;
    };
    
}

#endif
