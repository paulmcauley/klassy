/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "stylekitedynamic.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleKite18By18 : public RenderStyleKiteDynamic18By18
{
public:
    RenderStyleKite18By18(QPainter *painter,
                          const bool fromKstyle,
                          const bool boldButtonIcons,
                          const qreal devicePixelRatio,
                          const QPointF &deviceOffsetTitleBarTopLeftToIconTopLeft,
                          const bool forceEvenSquares)
        : RenderStyleKiteDynamic18By18(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetTitleBarTopLeftToIconTopLeft, forceEvenSquares) { };

    void renderMinimizeIcon() override
    {
        renderDynamicMinimizeIcon(false);
    }

private:
};

}
