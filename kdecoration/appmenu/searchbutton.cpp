/*
 * Copyright (C) 2025 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// own
#include "appmenu/searchbutton.h"

// Qt
#include <QPainter>
#include <QtMath>

namespace Breeze
{

AppMenuSearchButton::AppMenuSearchButton(Decoration *decoration, const int buttonIndex, QObject *parent)
    : AppMenuButton(decoration, buttonIndex, parent)
{
}

AppMenuSearchButton::~AppMenuSearchButton() = default;

void AppMenuSearchButton::paintIcon(QPainter *painter, const QRectF &iconRect, const qreal)
{
    // painter->setRenderHint(QPainter::Antialiasing, true);
    setPenWidth(painter, 1.25);

    const qreal circleRadius = 4.0;
    const QPointF circleCenter = iconRect.center();
    painter->drawEllipse(circleCenter, circleRadius, circleRadius);

    const qreal handleLength = 5.0;
    const qreal sqrt2 = qSqrt(2);
    const qreal handleAttachOffset = circleRadius / sqrt2;
    const QPointF handleStart = circleCenter + QPointF(handleAttachOffset, handleAttachOffset);
    const qreal handleEndOffset = (circleRadius + handleLength) / sqrt2;
    const QPointF handleEnd = circleCenter + QPointF(handleEndOffset, handleEndOffset);
    painter->drawLine(handleStart, handleEnd);
}

} // namespace Breeze
