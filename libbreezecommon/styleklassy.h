#ifndef BREEZE_STYLEKLASSY_H
#define BREEZE_STYLEKLASSY_H

/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleKlassy18By18 : public RenderDecorationButtonIcon18By18
{
public:
    RenderStyleKlassy18By18(QPainter *painter, const bool notInTitlebar, const bool boldButtonIcons, qreal devicePixelRatio, qreal iconScaleFactor)
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
    static constexpr qreal m_maximizeBoldPenWidthFactor = 1.5;

    //* how much to factor the pen width for a bold restore button
    static constexpr qreal m_restoreBoldPenWidthFactor = 1.5;
};

}

#endif
