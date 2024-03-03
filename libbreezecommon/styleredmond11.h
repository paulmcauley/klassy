/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "styleredmond10.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleRedmond1118By18 : public RenderStyleRedmond1018By18
{
public:
    /**
     * @brief Constructor - calls constructor of base class
     *
     * @param painter A QPainter object already initialised with an 18x18 reference window and pen.
     * @param fromKstyle Indicates that button is not to be drawn in the title bar, but somewhere else in the UI -- ususally means will be smaller
     * @param boldButtonIcons When in titlebar this will draw bolder button icons if true
     */
    RenderStyleRedmond1118By18(QPainter *painter,
                               const bool fromKstyle,
                               const bool boldButtonIcons,
                               const qreal devicePixelRatio,
                               const QPointF &deviceOffsetTitleBarTopLeftToIconTopLeft,
                               const bool forceEvenSquares)
        : RenderStyleRedmond1018By18(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetTitleBarTopLeftToIconTopLeft, forceEvenSquares){};

    void renderMaximizeIcon() override;
    void renderRestoreIcon() override;

private:
};

}
