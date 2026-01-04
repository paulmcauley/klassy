/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "stylekite.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleSuessigKite18By18 : public RenderStyleKite18By18
{
public:
    /**
     * @brief Constructor - calls constructor of base class
     *
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param fromKstyle Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
     * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
     */
    RenderStyleSuessigKite18By18(QPainter *painter,
                                 const bool fromKstyle,
                                 const bool boldButtonIcons,
                                 const qreal devicePixelRatio,
                                 const QPointF &deviceOffsetTitleBarTopLeftToIconTopLeft,
                                 const bool forceEvenSquares)
        : RenderStyleKite18By18(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetTitleBarTopLeftToIconTopLeft, forceEvenSquares) { };

    void renderMinimizeIcon() override;

private:
};

}
