#ifndef BREEZE_STYLECLASSIK_H
#define BREEZE_STYLECLASSIK_H

/*
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{
    
    class RenderStyleClassik18By18 : public RenderDecorationButtonIcon18By18
    {
        public:
            /**
             * @brief Constructor - calls constructor of base class
             * 
             * @param internalSettings An InternalSettingsPtr from the Window decoration config
             * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
             * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
             */
            RenderStyleClassik18By18(const QSharedPointer<InternalSettings>& internalSettings, QPainter* painter, const bool notInTitlebar) : RenderDecorationButtonIcon18By18(internalSettings, painter, notInTitlebar){};
            
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
