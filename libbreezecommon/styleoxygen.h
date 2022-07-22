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
    RenderStyleOxygen18By18(QPainter *painter, const bool notInTitlebar, const bool boldButtonIcons, const qreal devicePixelRatio, const bool iconScaleFactor)
        : RenderDecorationButtonIcon18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor){};

    void renderCloseIcon() override;
    void renderMaximizeIcon() override;
    void renderRestoreIcon() override;
    void renderMinimizeIcon() override;
    void renderKeepBehindIcon() override;
    void renderKeepInFrontIcon() override;
    void renderContextHelpIcon() override;
};

}

#endif
