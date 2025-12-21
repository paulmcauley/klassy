/*
 * SPDX-FileCopyrightText: 2025 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekairnleft.h"
#include <cmath>
#include <numbers>

namespace Breeze
{

void RenderStyleKairnLeft18By18::renderMinimizeIcon()
{
    // first determine the size of the maximize icon so the minimize icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);
    Q_UNUSED(maximizePenWidth);
    QPen pen = m_painter->pen();
    bool isOddPenWidth = true;

    if (!m_fromKstyle) {
        qreal roundedBoldPenWidth;
        if (m_boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, m_squareMaximizeBoldPenWidthFactor);
        } else
            isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth);
    }

    // make excessively thick pen widths translucent to balance with other buttons
    qreal originalOpacity = pen.color().alphaF();
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

    QVector<QPointF> line;
    // horizontal line
    // original y position in design was 12.5 -- this is often too high
    if (isOddPenWidth) {
        line = {snapToNearestPixel(QPointF(maximizeRect.left(), 4.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(maximizeRect.left(), 13.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down)};

    } else {
        line = {snapToNearestPixel(QPointF(maximizeRect.left(), 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(maximizeRect.left(), 13.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down)};
    }

    if (m_strokeToFilledPath) {
        QPainterPath path;
        path.addPolygon(line);
        QPainterPathStroker stroker(m_painter->pen());
        path = stroker.createStroke(path);
        m_painter->setBrush(m_painter->pen().color());
        m_painter->setPen(Qt::NoPen);
        m_painter->drawPath(path);
    } else {
        m_painter->drawPolyline(line);
    }

    // draw the arrow
    m_painter->setPen(Qt::NoPen);
    penColor.setAlphaF(originalOpacity);
    m_painter->setBrush(penColor);
    QPointF arrowTip(line[0].x() + pen.widthF() / 2, (line[1].y() + line[0].y()) / 2);
    qreal arrowRight(maximizeRect.center().x() + pen.widthF() / 2);
    arrowRight = roundCoordToWhole(arrowRight, ThresholdRound::Up);
    qreal arrowWidth = arrowRight - arrowTip.x();
    qreal halfArrowHeight = arrowWidth / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
    QPointF arrowTop(arrowRight, arrowTip.y() - halfArrowHeight);
    QPointF arrowBottom(arrowRight, arrowTip.y() + halfArrowHeight);
    QVector<QPointF> arrow{arrowTop, arrowTip, arrowBottom};
    m_painter->drawPolygon(arrow);
}

}
