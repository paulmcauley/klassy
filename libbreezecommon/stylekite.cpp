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

void RenderStyleKite18By18::renderRestoreIcon()
{
    // first determine the size of the maximize icon so the restore icon can align with it vertically
    QRectF maximizeRect = renderSquareMaximizeIcon(true);

    QPen pen = m_painter->pen();
    QPolygonF poly;

    if (m_fromKstyle) { // slightly smaller diamond
        pen.setJoinStyle(Qt::RoundJoin);
        m_painter->setPen(pen);

        // diamond / floating kite
        poly = QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)};

    } else {
        // thicker pen in titlebar
        if (m_boldButtonIcons) {
            pen.setWidthF(pen.widthF() * 1.75);
        }
        pen.setJoinStyle(Qt::RoundJoin);
        m_painter->setPen(pen);

        // diamond / floating kite
        poly = QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)};

        /* //original large diamond
        m_painter->drawConvexPolygon( QVector<QPointF>{
            QPointF( 4, 9 ),
            QPointF( 9, 4 ),
            QPointF( 14, 9 ),
            QPointF( 9, 14 )} );
        */
    }
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
    renderKeepBehindIconAsFromBreezeIcons();
}

void RenderStyleKite18By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
}

void RenderStyleKite18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}
}
