/*
 * SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

    struct Shadow {
        QPoint offset;
        int radius;
        QColor color;
    };

    QVector<Shadow> m_shadows;
};

} // namespace Breeze
