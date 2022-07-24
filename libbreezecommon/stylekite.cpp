/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekite.h"

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
    if (m_fromKstyle) { // slightly smaller diamond
        m_pen.setJoinStyle(Qt::RoundJoin);
        m_painter->setPen(m_pen);

        // diamond / floating kite
        m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)});

    } else {
        // thicker pen in titlebar
        if (m_boldButtonIcons) {
            m_pen.setWidthF(m_pen.widthF() * 1.75);
        }
        m_pen.setJoinStyle(Qt::RoundJoin);
        m_painter->setPen(m_pen);

        // diamond / floating kite
        m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)});

        /* //original large diamond
        m_painter->drawConvexPolygon( QVector<QPointF>{
            QPointF( 4, 9 ),
            QPointF( 9, 4 ),
            QPointF( 14, 9 ),
            QPointF( 9, 14 )} );
        */
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
