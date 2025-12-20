/*
 * SPDX-FileCopyrightText: 2025 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekairn.h"

namespace Breeze
{

void RenderStyleKairn18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKairn18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 0.025, true);
}

void RenderStyleKairn18By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon(true);
}

void RenderStyleKairn18By18::renderMinimizeIcon()
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

    // draw the arrow
    m_painter->setPen(Qt::NoPen);
    penColor.setAlphaF(originalOpacity);
    m_painter->setBrush(penColor);
    QPointF arrowTip((line[1].x() + line[0].x()) / 2, line[0].y() - pen.widthF() / 2);
    qreal arrowTop(maximizeRect.center().y() - pen.widthF() / 2);
    arrowTop = roundCoordToWhole(arrowTop, ThresholdRound::Down);
    qreal halfArrowLength = (arrowTip.y() - arrowTop) / 2;
    QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowTop);
    QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowTop);
    QVector<QPointF> arrow{arrowLeft, arrowTip, arrowRight};
    m_painter->drawPolygon(arrow);
}

// For consistency with breeze icon set
void RenderStyleKairn18By18::renderKeepBehindIcon()
{
    // renderKeepBehindIconAsFromBreezeIcons();
    // renderOverlappingWindowsIcon(false, -1, false, true);

    QPen pen = m_painter->pen();
    if (m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * m_squareMaximizeBoldPenWidthFactor);
    }
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPainterPath outerRing;
    outerRing.addEllipse(QRectF(3, 3, 12, 12));
    m_painter->drawPath(outerRing);

    QPainterPath innerX;
    innerX.moveTo(7, 7);
    innerX.lineTo(11, 11);
    innerX.moveTo(11, 7);
    innerX.lineTo(7, 11);
    m_painter->drawPath(innerX);
}

void RenderStyleKairn18By18::renderKeepInFrontIcon()
{
    // renderKeepInFrontIconAsFromBreezeIcons();
    // renderOverlappingWindowsIcon(false, -1, true, false);

    QPen pen = m_painter->pen();
    if (m_boldButtonIcons) {
        pen.setWidthF(pen.widthF() * m_squareMaximizeBoldPenWidthFactor);
    }
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPainterPath outerRing;
    outerRing.addEllipse(QRectF(3, 3, 12, 12));
    m_painter->drawPath(outerRing);

    QPainterPath innerDot;
    innerDot.addEllipse(QRectF(7.5, 7.5, 3, 3));
    m_painter->setBrush(color);
    m_painter->setPen(Qt::NoPen);
    m_painter->drawPath(innerDot);
}

void RenderStyleKairn18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

// don't render pinned differently to pin so that this icon is not confused with KeepInFront icon
void RenderStyleKairn18By18::renderPinnedOnAllDesktopsIcon()
{
    renderPinOnAllDesktopsIcon();
}

void RenderStyleKairn18By18::renderShadeIcon()
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
    if (isOddPenWidth) {
        line = {snapToNearestPixel(QPointF(4.5, maximizeRect.top()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(13.5, maximizeRect.top()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down)};

    } else {
        line = {snapToNearestPixel(QPointF(4.5, maximizeRect.top()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(13.5, maximizeRect.top()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down)};
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
    QPointF arrowTip((line[1].x() + line[0].x()) / 2, line[0].y() + pen.widthF() / 2);
    qreal arrowBottom(maximizeRect.center().y() + pen.widthF() / 2);
    arrowBottom = roundCoordToWhole(arrowBottom, ThresholdRound::Up);
    qreal halfArrowLength = (arrowBottom - arrowTip.y()) / 2;
    QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowBottom);
    QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowBottom);
    QVector<QPointF> arrow{arrowLeft, arrowTip, arrowRight};
    m_painter->drawPolygon(arrow);
}

void RenderStyleKairn18By18::renderUnShadeIcon()
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
    if (isOddPenWidth) {
        line = {snapToNearestPixel(QPointF(4.5, maximizeRect.top()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(13.5, maximizeRect.top()), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down)};

    } else {
        line = {snapToNearestPixel(QPointF(4.5, maximizeRect.top()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down),
                snapToNearestPixel(QPointF(13.5, maximizeRect.top()), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down)};
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
    qreal arrowTop(line[0].y() + pen.widthF() * 2);
    arrowTop = roundCoordToWhole(arrowTop, ThresholdRound::Up);
    QPointF arrowTip((line[1].x() + line[0].x()) / 2, maximizeRect.center().y() + pen.widthF() * 2.5);
    qreal halfArrowLength = (arrowTip.y() - arrowTop) / 2;
    QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowTop);
    QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowTop);
    QVector<QPointF> arrow{arrowLeft, arrowTip, arrowRight};
    m_painter->drawPolygon(arrow);
}

}
