#ifndef BREEZE_STYLESYSTEMICONTHEME_H
#define BREEZE_STYLESYSTEMICONTHEME_H

/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleSystemIconTheme : public RenderDecorationButtonIcon18By18
{
public:
    /**
     * @brief Constructor - calls constructor of base class
     *
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
     * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
     */
    RenderStyleSystemIconTheme(QPainter *painter,
                               const bool notInTitlebar,
                               const bool boldButtonIcons,
                               qreal iconWidth,
                               const QSharedPointer<InternalSettings> internalSettings,
                               qreal devicePixelRatio,
                               qreal iconScaleFactor)
        : RenderDecorationButtonIcon18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor)
        , m_iconWidth{iconWidth}
        , m_internalSettings{internalSettings} {};

    void renderCloseIcon() override;
    void renderMaximizeIcon() override;
    void renderRestoreIcon() override;
    void renderMinimizeIcon() override;
    void renderKeepBehindIcon() override;
    void renderKeepInFrontIcon() override;
    void renderContextHelpIcon() override;
    void renderShadeIcon() override;
    void renderUnShadeIcon() override;

private:
    void paintIconFromSystemTheme(QString iconName);

    qreal m_iconWidth;
    const QSharedPointer<InternalSettings> m_internalSettings;
};

}

#endif
