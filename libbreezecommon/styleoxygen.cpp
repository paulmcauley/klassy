/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleoxygen.h"

namespace Breeze
{
void RenderStyleOxygen18By18::renderCloseIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderCloseIcon();
}

void RenderStyleOxygen18By18::renderMaximizeIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderMaximizeIcon();
}

void RenderStyleOxygen18By18::renderFloatIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderFloatIcon();
}

void RenderStyleOxygen18By18::renderMinimizeIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.75);
        m_painter->setPen(pen);
    }

    RenderDecorationButtonIcon18By18::renderMinimizeIcon();
}

void RenderStyleOxygen18By18::renderPinnedOnAllDesktopsIcon()
{
    QPen pen = m_painter->pen();
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    m_painter->setBrush(color);
    m_painter->setPen(Qt::NoPen);

    QPainterPath outerRing;
    outerRing.addEllipse(QRectF(3, 3, 12, 12));

    QPainterPath innerDot;
    innerDot.addEllipse(QRectF(8, 8, 2, 2));

    outerRing = outerRing.subtracted(innerDot);

    m_painter->drawPath(outerRing);
}

void RenderStyleOxygen18By18::renderPinOnAllDesktopsIcon()
{
    QPen pen = m_painter->pen();
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    m_painter->setBrush(color);
    m_painter->setPen(Qt::NoPen);
    m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(6.5, 8.5), QPointF(12, 3), QPointF(15, 6), QPointF(9.5, 11.5)});

    pen.setColor(color);
    m_painter->setPen(pen);
    QVector<QPointF> line1{QPointF(5.5, 7.5), QPointF(10.5, 12.5)};
    QVector<QPointF> line2{QPointF(8, 10), QPointF(4.5, 13.5)};

    if (m_strokeToFilledPath) {
        QPainterPath path1, path2;
        path1.addPolygon(line1);
        path2.addPolygon(line2);
        QPainterPathStroker stroker(m_painter->pen());
        path1 = stroker.createStroke(path1);
        path2 = stroker.createStroke(path2);
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(pen.color());
        m_painter->drawPath(path1);
        m_painter->drawPath(path2);
    } else {
        m_painter->drawPolyline(line1);
        m_painter->drawPolyline(line2);
    }
}

void RenderStyleOxygen18By18::renderKeepBehindIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.4);
        m_painter->setPen(pen);
    }

    QVector<QPointF> line1, line2;

    // two down arrows
    line1 = {QPointF(4, 5), QPointF(9, 10), QPointF(14, 5)};
    line2 = {QPointF(4, 9), QPointF(9, 14), QPointF(14, 9)};

    if (m_strokeToFilledPath) {
        QPainterPath path1, path2;
        path1.addPolygon(line1);
        path2.addPolygon(line2);
        QPainterPathStroker stroker(m_painter->pen());
        path1 = stroker.createStroke(path1);
        path2 = stroker.createStroke(path2);
        m_painter->setBrush(m_painter->pen().color());
        m_painter->setPen(Qt::NoPen);
        m_painter->drawPath(path1);
        m_painter->drawPath(path2);
    } else {
        m_painter->drawPolyline(line1);
        m_painter->drawPolyline(line2);
    }
}

void RenderStyleOxygen18By18::renderKeepInFrontIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * 1.4);
        m_painter->setPen(pen);
    }

    QVector<QPointF> line1, line2;

    // two up arrows
    line1 = {QPointF(4, 9), QPointF(9, 4), QPointF(14, 9)};
    line2 = {QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)};

    if (m_strokeToFilledPath) {
        QPainterPath path1, path2;
        path1.addPolygon(line1);
        path2.addPolygon(line2);
        QPainterPathStroker stroker(m_painter->pen());
        path1 = stroker.createStroke(path1);
        path2 = stroker.createStroke(path2);
        m_painter->setBrush(m_painter->pen().color());
        m_painter->setPen(Qt::NoPen);
        m_painter->drawPath(path1);
        m_painter->drawPath(path2);
    } else {
        m_painter->drawPolyline(line1);
        m_painter->drawPolyline(line2);
    }
}

}
