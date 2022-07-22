#ifndef BREEZE_STYLEREDMOND10_H
#define BREEZE_STYLEREDMOND10_H

/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleRedmond1018By18 : public RenderDecorationButtonIcon18By18
{
public:
    /**
     * @brief Constructor - calls constructor of base class
     *
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param notInTitlebar Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
     * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
     */
    RenderStyleRedmond1018By18(QPainter *painter,
                               const bool notInTitlebar,
                               const bool boldButtonIcons,
                               const qreal devicePixelRatio,
                               const bool iconScaleFactor)
        : RenderDecorationButtonIcon18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor){};

    void renderCloseIcon() override;
    void renderMaximizeIcon() override;
    void renderRestoreIcon() override;
    void renderMinimizeIcon() override;
    void renderKeepBehindIcon() override;
    void renderKeepInFrontIcon() override;
    void renderContextHelpIcon() override;

private:
    void renderRestoreIconAfterPenWidthSet();

    //* how much to factor the pen width for a bold maximize button
    static constexpr qreal m_maximizeBoldPenWidthFactor = 1.666; // 1.5;

    //* how much to factor the pen width for a bold restore button
    static constexpr qreal m_restoreBoldPenWidthFactor = 1.5;
};

}

#endif
