/*
 * SPDX-FileCopyrightText: 2026 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekisweetdynamic.h"
#include <cmath>
#include <numbers>

namespace Breeze
{

void RenderStyleKisweetDynamic18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleKisweetDynamic18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false, 0.025, true);
}

void RenderStyleKisweetDynamic18By18::renderFloatIcon()
{
    renderOverlappingWindowsIcon(true);
}

void RenderStyleKisweetDynamic18By18::renderMinimizeIcon()
{
    if ((m_taskManagerSide == SideLeft || m_taskManagerSide == SideRight) && m_taskManagerType == TaskManagerType::IconsOnlyTaskManager) {
        renderTinySquareMinimizeIcon(true);
        return;
    }

    // first determine the size of the maximize icon so the minimize icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QVector<QPointF> line;
    switch (m_taskManagerSide) {
    case SideBottom:
    default:
        line.append(maximizeRect.bottomLeft());
        line.append(maximizeRect.bottomRight());
        break;
    case SideLeft:
        line.append(maximizeRect.topLeft());
        line.append(maximizeRect.bottomLeft());
        break;
    case SideTop:
        line.append(maximizeRect.topLeft());
        line.append(maximizeRect.topRight());
        break;
    case SideRight:
        line.append(maximizeRect.topRight());
        line.append(maximizeRect.bottomRight());
        break;
    }

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);

    // make excessively thick pen widths translucent to balance with other buttons
    qreal originalOpacity = pen.color().alphaF();
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

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

    QVector<QPointF> arrow;
    switch (m_taskManagerSide) {
    case SideBottom:
    default: {
        QPointF arrowTip((line[1].x() + line[0].x()) / 2, line[0].y() - penWidthLocal / 2);
        qreal arrowTop(maximizeRect.center().y() - penWidthLocal / 2);
        arrowTop = roundCoordToWhole(arrowTop, ThresholdRound::Down);
        qreal arrowHeight = arrowTip.y() - arrowTop;
        qreal halfArrowLength = arrowHeight / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
        QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowTop);
        QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowTop);
        arrow = {arrowLeft, arrowTip, arrowRight};
        break;
    }
    case SideLeft: {
        QPointF arrowTip(line[0].x() + penWidthLocal / 2, (line[1].y() + line[0].y()) / 2);
        qreal arrowRight(maximizeRect.center().x() + penWidthLocal / 2);
        arrowRight = roundCoordToWhole(arrowRight, ThresholdRound::Up);
        qreal arrowWidth = arrowRight - arrowTip.x();
        qreal halfArrowHeight = arrowWidth / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
        QPointF arrowTop(arrowRight, arrowTip.y() - halfArrowHeight);
        QPointF arrowBottom(arrowRight, arrowTip.y() + halfArrowHeight);
        arrow = {arrowTop, arrowTip, arrowBottom};
        break;
    }
    case SideTop: {
        QPointF arrowTip((line[1].x() + line[0].x()) / 2, line[0].y() + penWidthLocal / 2);
        qreal arrowBottom(maximizeRect.center().y() + penWidthLocal / 2);
        arrowBottom = roundCoordToWhole(arrowBottom, ThresholdRound::Up);
        qreal arrowHeight = arrowBottom - arrowTip.y();
        qreal halfArrowLength = arrowHeight / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
        QPointF arrowLeft(arrowTip.x() - halfArrowLength, arrowBottom);
        QPointF arrowRight(arrowTip.x() + halfArrowLength, arrowBottom);
        arrow = {arrowLeft, arrowTip, arrowRight};
        break;
    }
    case SideRight: {
        QPointF arrowTip(line[0].x() - penWidthLocal / 2, (line[1].y() + line[0].y()) / 2);
        qreal arrowLeft(maximizeRect.center().x() - penWidthLocal / 2);
        arrowLeft = roundCoordToWhole(arrowLeft, ThresholdRound::Down);
        qreal arrowWidth = arrowTip.x() - arrowLeft;
        qreal halfArrowHeight = arrowWidth / (std::tan(std::numbers::pi / 3)); // make equilateral triangle
        QPointF arrowTop(arrowLeft, arrowTip.y() - halfArrowHeight);
        QPointF arrowBottom(arrowLeft, arrowTip.y() + halfArrowHeight);
        arrow = {arrowTop, arrowTip, arrowBottom};
        break;
    }
    }

    m_painter->drawPolygon(arrow);
}

void RenderStyleKisweetDynamic18By18::renderShadeIcon()
{
    // first determine the size of the maximize icon so the minimize icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QVector<QPointF> line = {maximizeRect.topLeft(), maximizeRect.topRight()};

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);

    // make excessively thick pen widths translucent to balance with other buttons
    qreal originalOpacity = pen.color().alphaF();
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

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

void RenderStyleKisweetDynamic18By18::renderUnShadeIcon()
{
    // first determine the size of the maximize icon so the minimize icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QVector<QPointF> line = {maximizeRect.topLeft(), maximizeRect.topRight()};

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);

    // make excessively thick pen widths translucent to balance with other buttons
    qreal originalOpacity = pen.color().alphaF();
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

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
