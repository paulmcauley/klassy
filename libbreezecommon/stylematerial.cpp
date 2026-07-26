/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylematerial.h"
#include <QPainterPathStroker>

namespace Breeze
{

void RenderStyleMaterial18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleMaterial18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleMaterial18By18::renderFloatIcon()
{
    // first determine the size of the maximize icon so the restore icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QPen pen = m_painter->pen();

    // make excessively thick pen widths translucent to balance with other buttons
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    pen.setWidthF(maximizePenWidth);
    qreal devicePenWidth = penWidthToDevice(pen);
    bool isOddPenWidth(qRound(devicePenWidth) % 2 != 0);

    m_painter->setPen(pen);

    QPainterPath squarePath, arrowPath;

    qreal squareWidth = maximizeRect.width() * 0.7;
    qreal arrowLength = maximizeRect.width() * 0.6;

    QRectF square;
    QPolygonF arrow;

    QPointF squareTopLeft;
    QPointF arrowTopLeft;
    if (isOddPenWidth) {
        squareTopLeft = snapToNearestPixel(QPointF(maximizeRect.right() - squareWidth, maximizeRect.top()), SnapPixel::ToHalf, SnapPixel::ToHalf);
        arrowTopLeft = snapToNearestPixel(QPointF(maximizeRect.left(), maximizeRect.bottom() - arrowLength), SnapPixel::ToHalf, SnapPixel::ToHalf);
    } else {
        squareTopLeft = snapToNearestPixel(QPointF(maximizeRect.right() - squareWidth, maximizeRect.top()), SnapPixel::ToWhole, SnapPixel::ToWhole);
        arrowTopLeft = snapToNearestPixel(QPointF(maximizeRect.left(), maximizeRect.bottom() - arrowLength), SnapPixel::ToWhole, SnapPixel::ToWhole);
    }

    qreal squareWidthSnapped = maximizeRect.right() - squareTopLeft.x();
    QPointF squareBottomRight(maximizeRect.right(), maximizeRect.top() + squareWidthSnapped);
    square = {squareTopLeft, squareBottomRight};

    qreal arrowWidthSnapped = maximizeRect.bottom() - arrowTopLeft.y();
    QPointF arrowBottomRight(maximizeRect.left() + arrowWidthSnapped, maximizeRect.bottom());
    arrow << arrowTopLeft << maximizeRect.bottomLeft() << arrowBottomRight;

    // shrink square if gap between square and arrow is too small
    qreal localPenWidth = penWidthToLocal(pen);
    qreal onePixelLocal = convertDevicePixelsToLocal(1);
    if ((square.left() - arrow[0].x() - localPenWidth) < onePixelLocal) {
        square.setBottomLeft(QPointF(square.left() + onePixelLocal, square.bottom() - onePixelLocal));
    }

    squarePath.addRect(square);
    arrowPath.addPolygon(arrow);

    if (m_strokeToFilledPath) {
        QPainterPathStroker stroker(m_painter->pen());
        squarePath = stroker.createStroke(squarePath);
        arrowPath = stroker.createStroke(arrowPath);
        m_painter->setBrush(m_painter->pen().color());
        m_painter->setPen(Qt::NoPen);
    }
    m_painter->drawPath(squarePath);
    m_painter->drawPath(arrowPath);
}

void RenderStyleMaterial18By18::renderMinimizeIcon()
{
    // first determine the size of the maximize icon so the minimize icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);
    bool isOddPenWidth(qRound(penWidthToDevice(pen)) % 2 != 0);

    // make excessively thick pen widths translucent to balance with other buttons
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

    QVector<QPointF> line;
    // horizontal line
    if (isOddPenWidth) {
        line = {snapToNearestPixel(QPointF(4.5, maximizeRect.bottom()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(13.5, maximizeRect.bottom()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down)};

    } else {
        line = {snapToNearestPixel(QPointF(4.5, maximizeRect.bottom()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(13.5, maximizeRect.bottom()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down)};
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
}
}
