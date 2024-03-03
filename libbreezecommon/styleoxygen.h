/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "renderdecorationbuttonicon18by18.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleOxygen18By18 : public RenderDecorationButtonIcon18By18
{
public:
    RenderStyleOxygen18By18(QPainter *painter,
                            const bool fromKstyle,
                            const bool boldButtonIcons,
                            const qreal devicePixelRatio,
                            const QPointF &deviceOffsetTitleBarTopLeftToIconTopLeft,
                            const bool forceEvenSquares)
        : RenderDecorationButtonIcon18By18(painter,
                                           fromKstyle,
                                           boldButtonIcons,
                                           devicePixelRatio,
                                           deviceOffsetTitleBarTopLeftToIconTopLeft,
                                           forceEvenSquares){};

    void renderCloseIcon() override;
    void renderMaximizeIcon() override;
    void renderRestoreIcon() override;
    void renderMinimizeIcon() override;
    void renderKeepBehindIcon() override;
    void renderKeepInFrontIcon() override;
    void renderContextHelpIcon() override;
};

}
