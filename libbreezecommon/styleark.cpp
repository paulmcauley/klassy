/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleark.h"
#include <QPainterPath>
#include <QPainterPathStroker>

namespace Breeze
{

void RenderStyleArk18By18::renderMinimizeIcon()
{
    m_painter->setBrush(m_painter->pen().color());
    m_painter->setPen(Qt::PenStyle::NoPen);
    QRectF rect(snapToNearestPixel(QPointF(6, 8)), snapToNearestPixel(QPointF(12, 10)));
    m_painter->drawRect(rect);
}

void RenderStyleArk18By18::renderMaximizeIcon()
{
    m_painter->setBrush(m_painter->pen().color());
    m_painter->setPen(Qt::PenStyle::NoPen);

    qreal top, right, bottom, left;
    top = roundCoordToWhole(5);
    right = roundCoordToWhole(13);
    bottom = roundCoordToWhole(13);
    left = roundCoordToWhole(5);

    QPainterPath bottomLeftArrow, topRightArrow;

    bottomLeftArrow.moveTo(left, 7.5);
    bottomLeftArrow.lineTo(left, 12);
    qreal cornerRadius = bottom - 12;
    QRectF blCornerRect(QPointF(left, 12 - cornerRadius), QPointF(left + cornerRadius * 2, bottom));
    bottomLeftArrow.arcTo(blCornerRect, 180, 90);
    bottomLeftArrow.lineTo(10.5, bottom);
    bottomLeftArrow.closeSubpath();
    m_painter->drawPath(bottomLeftArrow);

    topRightArrow.moveTo(7.5, top);
    topRightArrow.lineTo(12, top);
    cornerRadius = right - 12;
    QRectF trCornerRect(QPointF(right - 2 * cornerRadius, top), QPointF(right, top + 2 * cornerRadius));
    topRightArrow.arcTo(trCornerRect, 90, -90);
    topRightArrow.lineTo(right, 10.5);
    topRightArrow.closeSubpath();
    m_painter->drawPath(topRightArrow);
}

void RenderStyleArk18By18::renderFloatIcon()
{
    m_painter->setBrush(m_painter->pen().color());
    m_painter->setPen(Qt::PenStyle::NoPen);

    qreal top, right;
    top = roundCoordToWhole(9);
    right = roundCoordToWhole(9);
    qreal &bottom = top;
    qreal &left = right;

    QPainterPath bottomLeftArrow, topRightArrow;

    bottomLeftArrow.moveTo(4.5, top);
    bottomLeftArrow.lineTo(8, top);
    qreal cornerRadius = right - 8;
    QRectF trCornerRect(QPointF(right - cornerRadius * 2, top), QPointF(right, top + cornerRadius * 2));
    bottomLeftArrow.arcTo(trCornerRect, 90, -90);
    bottomLeftArrow.lineTo(right, 13.5);
    bottomLeftArrow.closeSubpath();
    m_painter->drawPath(bottomLeftArrow);

    topRightArrow.moveTo(left, 4.5);
    topRightArrow.lineTo(left, 8);
    cornerRadius = bottom - 8;
    QRectF blCornerRect(QPointF(left, bottom - 2 * cornerRadius), QPointF(left + 2 * cornerRadius, bottom));
    topRightArrow.arcTo(blCornerRect, 180, 90);
    topRightArrow.lineTo(13.5, bottom);
    topRightArrow.closeSubpath();
    m_painter->drawPath(topRightArrow);
}

void RenderStyleArk18By18::renderCloseIcon()
{
    QPainterPath x;
    x.moveTo(6, 6);
    x.lineTo(12, 12);
    x.moveTo(12, 6);
    x.lineTo(6, 12);

    QPen pen = m_painter->pen();
    pen.setCapStyle(Qt::PenCapStyle::RoundCap);
    pen.setWidthF(1.7);
    QPainterPathStroker stroker(pen);
    QPainterPath strokedX = stroker.createStroke(x);

    m_painter->setPen(Qt::PenStyle::NoPen);
    m_painter->setBrush(pen.color());
    m_painter->drawPath(strokedX);
}

}
