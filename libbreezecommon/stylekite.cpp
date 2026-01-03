/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekite.h"
#include <QPainterPathStroker>

namespace Breeze
{
void RenderStyleKite18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKite18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleKite18By18::renderFloatIcon()
{
    // first determine the size of the maximize icon so the float icon can align with it vertically
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);
    pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(pen);
    QPolygonF poly;
    // floating kite
    poly = QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)};

    // centre
    QPointF centerTranslate = QPointF(9, maximizeRect.center().y()) - poly.boundingRect().center();
    poly.translate(centerTranslate);

    if (m_strokeToFilledPath) {
        QPainterPath path;
        path.addPolygon(poly);
        path.closeSubpath();
        QPainterPathStroker stroker(m_painter->pen());
        path = stroker.createStroke(path);
        m_painter->setBrush(m_painter->pen().color());
        m_painter->setPen(Qt::NoPen);
        m_painter->drawPath(path);
    } else {
        m_painter->drawConvexPolygon(poly);
    }
}

void RenderStyleKite18By18::renderMinimizeIcon()
{
    renderTinySquareMinimizeIcon();
}

// For consistency with breeze icon set
void RenderStyleKite18By18::renderKeepBehindIcon()
{
    renderSelectedXKeepBehindIcon();
}

void RenderStyleKite18By18::renderKeepInFrontIcon()
{
    renderSelectedDotKeepInFrontIcon();
}

void RenderStyleKite18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}
}
