/*
 * SPDX-FileCopyrightText: 2025 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekopal.h"
#include <cmath>
#include <numbers>

namespace Breeze
{

void RenderStyleKopal18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKopal18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 0.025, true);
}

void RenderStyleKopal18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(true);
}

void RenderStyleKopal18By18::renderMinimizeIcon()
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
    qreal penWidthLocal = penWidthToLocal(pen);
    QPointF arrowTip((line[1].x() + line[0].x()) / 2, line[0].y() - penWidthLocal / 2);
    qreal arrowTop(maximizeRect.center().y() - penWidthLocal / 2);
    arrowTop = roundCoordToWhole(arrowTop, ThresholdRound::Down);
    qreal arrowHeight = arrowTip.y() - arrowTop;
    qreal halfArrowLength = arrowHeight / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
    QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowTop);
    QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowTop);
    QVector<QPointF> arrow{arrowLeft, arrowTip, arrowRight};
    m_painter->drawPolygon(arrow);
}

void RenderStyleKopal18By18::renderKeepBehindIcon()
{
    renderSelectedXKeepBehindIcon();
}

void RenderStyleKopal18By18::renderKeepInFrontIcon()
{
    renderSelectedDotKeepInFrontIcon();
}

void RenderStyleKopal18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

void RenderStyleKopal18By18::renderPinOnAllDesktopsIcon()
{
    renderOnAllDesktopsIcon();
}

void RenderStyleKopal18By18::renderPinnedOnAllDesktopsIcon()
{
    renderOnAllDesktopsIcon();
}

void RenderStyleKopal18By18::renderShadeIcon()
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

    qreal penWidthLocal = penWidthToLocal(pen);
    QPointF arrowTip((line[1].x() + line[0].x()) / 2, line[0].y() + penWidthLocal / 2);
    qreal arrowBottom(maximizeRect.center().y() + penWidthLocal / 2);
    arrowBottom = roundCoordToWhole(arrowBottom, ThresholdRound::Up);
    qreal arrowHeight = arrowBottom - arrowTip.y();
    qreal halfArrowLength = arrowHeight / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
    QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowBottom);
    QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowBottom);
    QVector<QPointF> arrow{arrowLeft, arrowTip, arrowRight};
    m_painter->drawPolygon(arrow);
}

void RenderStyleKopal18By18::renderUnShadeIcon()
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
    qreal penWidthLocal = penWidthToLocal(pen);
    qreal arrowTop(line[0].y() + penWidthLocal * 2);
    arrowTop = roundCoordToWhole(arrowTop, ThresholdRound::Up);
    QPointF arrowTip((line[1].x() + line[0].x()) / 2, maximizeRect.center().y() + penWidthLocal * 2.5);
    qreal arrowHeight = arrowTip.y() - arrowTop;
    qreal halfArrowLength = arrowHeight / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
    QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowTop);
    QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowTop);
    QVector<QPointF> arrow{arrowLeft, arrowTip, arrowRight};
    m_painter->drawPolygon(arrow);
}

}
