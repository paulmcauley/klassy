/*
 * Copyright (C) 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

// own
#include "breezecommon_export.h"

// Qt
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QSize>

namespace Breeze
{

class BREEZECOMMON_EXPORT BoxShadowRenderer
{
public:
    // Compiler generated constructors & destructor are fine.

    /**
     * Set the size of the box.
     * @param size The size of the box.
     **/
    void setBoxSize(const QSize &size);

    /**
     * Set the radius of box' corners.
     * @param radius The border radius, in pixels.
     **/
    void setBorderRadius(qreal radius);

    /**
     * Set the device pixel ratio of the resulting shadow texture.
     * @param dpr The device pixel ratio.
     **/
    void setDevicePixelRatio(qreal dpr);

    /**
     * Add a shadow.
     * @param offset The offset of the shadow.
     * @param radius The blur radius.
     * @param color The color of the shadow.
     **/
    void addShadow(const QPoint &offset, int radius, const QColor &color);

    /**
     * Render the shadow.
     **/
    QImage render() const;

    /**
     * Calculate the minimum size of the box.
     *
     * This helper computes the minimum size of the box so the shadow behind it has
     * full its strength.
     *
     * @param radius The blur radius of the shadow.
     **/
    static QSize calculateMinimumBoxSize(int radius);

    /**
     * Calculate the minimum size of the shadow texture.
     *
     * This helper computes the minimum size of the resulting texture so the shadow
     * is not clipped.
     *
     * @param boxSize The size of the box.
     * @param radius The blur radius.
     * @param offset The offset of the shadow.
     **/
    static QSize calculateMinimumShadowTextureSize(const QSize &boxSize, int radius, const QPoint &offset);

private:
    QSize m_boxSize;
    qreal m_borderRadius = 0.0;
    qreal m_dpr = 1.0;

    struct Shadow {
        QPoint offset;
        int radius;
        QColor color;
    };

    QVector<Shadow> m_shadows;
};

} // namespace Breeze
