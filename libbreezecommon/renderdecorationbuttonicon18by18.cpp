/*
 * SPDX-FileCopyrightText: 2022-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon18by18.h"
#include <QGraphicsItem>
#include <QPainterPathStroker>
#include <cmath>

namespace Klassy
{

RenderDecorationButtonIcon18By18::RenderDecorationButtonIcon18By18(QPainter *painter)
    : RenderDecorationButtonIcon(painter)
{
}

/* most base methods here are KDE's default Breeze/Oxygen style -- override with other styles */
void RenderDecorationButtonIcon18By18::renderCloseIcon()
{
    QVector<QPointF> line1{QPointF(5, 5), QPointF(13, 13)};
    QVector<QPointF> line2{QPointF(13, 5), QPointF(5, 13)};

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

void RenderDecorationButtonIcon18By18::renderMaximizeIcon()
{
    // up arrow
    QVector<QPointF> line{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)};

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

void RenderDecorationButtonIcon18By18::renderFloatIcon()
{
    QPen pen = m_painter->pen();
    pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(pen);

    QVector<QPointF> diamond;
    if (m_fromKstyle) { // slightly smaller diamond
        // diamond / floating kite
        diamond = QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)};

    } else {
        // diamond / floating kite
        diamond = QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9), QPointF(9, 14)};
    }

    if (m_strokeToFilledPath) {
        QPainterPath path;
        path.addPolygon(diamond);
        path.closeSubpath();
        QPainterPathStroker stroker(m_painter->pen());
        path = stroker.createStroke(path);
        m_painter->setBrush(m_painter->pen().color());
        m_painter->setPen(Qt::NoPen);
        m_painter->drawPath(path);
    } else {
        m_painter->drawConvexPolygon(diamond);
    }
}

void RenderDecorationButtonIcon18By18::renderMinimizeIcon()
{
    // down arrow
    QVector<QPointF> line{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)};

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

void RenderDecorationButtonIcon18By18::renderCenteredLineMinimizeIcon(bool smaller)
{
    QPen pen = m_painter->pen();

    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);
    pen.setWidthF(maximizePenWidth);
    bool isOddPenWidth(roundedPenWidthIsOdd(pen));

    // make excessively thick pen widths translucent to balance with other buttons
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);

    m_painter->setPen(pen);

    QVector<QPointF> line;
    qreal x1, x2, y;
    if (smaller) {
        x1 = 5.5;
        x2 = 12.5;
    } else {
        x1 = maximizeRect.left();
        x2 = maximizeRect.right();
    }
    y = maximizeRect.center().y();

    // horizontal line
    if (isOddPenWidth) {
        line = {snapToNearestPixel(QPointF(x1, y), SnapPixel::ToHalf, SnapPixel::ToHalf),
                snapToNearestPixel(QPointF(x2, y), SnapPixel::ToHalf, SnapPixel::ToHalf)};

    } else {
        line = {snapToNearestPixel(QPointF(x1, y), SnapPixel::ToWhole, SnapPixel::ToWhole),
                snapToNearestPixel(QPointF(x2, y), SnapPixel::ToWhole, SnapPixel::ToWhole)};
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

void RenderDecorationButtonIcon18By18::renderDynamicMinimizeIcon(bool dynamicMinimize)
{
    QVector<QPointF> line;
    QPen pen = m_painter->pen();

    if (!dynamicMinimize) { // bottom line
        auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);
        line.append(maximizeRect.bottomLeft());
        line.append(maximizeRect.bottomRight());
        pen.setWidthF(maximizePenWidth);
    } else { // dynamic minimize icon
        switch (m_taskManagerSide) {
        case SideBottom:
        default: {
            auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);
            line.append(maximizeRect.bottomLeft());
            line.append(maximizeRect.bottomRight());
            pen.setWidthF(maximizePenWidth);
            break;
        }
        case SideLeft:
            (m_taskManagerType == TaskManagerType::IconsOnlyTaskManager) ? renderTinySquareMinimizeIcon() : renderCenteredLineMinimizeIcon();
            return;
        case SideTop: {
            auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);
            line.append(maximizeRect.topLeft());
            line.append(maximizeRect.topRight());
            pen.setWidthF(maximizePenWidth);
            break;
        }
        case SideRight:
            (m_taskManagerType == TaskManagerType::IconsOnlyTaskManager) ? renderTinySquareMinimizeIcon() : renderCenteredLineMinimizeIcon();
            return;
        }
    }

    // make excessively thick pen widths translucent to balance with other buttons
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
}

void RenderDecorationButtonIcon18By18::renderPinnedOnAllDesktopsIcon()
{
    renderOnAllDesktopsIcon();
}

void RenderDecorationButtonIcon18By18::renderPinOnAllDesktopsIcon()
{
    renderOnAllDesktopsIcon();
}

void RenderDecorationButtonIcon18By18::renderOnAllDesktopsIcon()
{
    QPen pen = m_painter->pen();
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    QBrush brush(color);
    pen = Qt::PenStyle::NoPen;
    m_painter->setBrush(brush);
    m_painter->setPen(pen);

    QPainterPath topLeftWorkspace, topRightWorkspace, bottomRightWorkspace, bottomLeftWorkspace;
    QRectF topLeftWorkspaceRect, topRightWorkspaceRect, bottomRightWorkspaceRect, bottomLeftWorkspaceRect;

    topLeftWorkspaceRect = QRectF(snapToNearestPixel(QPointF(3, 4)), snapToNearestPixel(QPointF(8, 8)));

    topRightWorkspaceRect = QRectF(snapToNearestPixel(QPointF(10, 4)), snapToNearestPixel(QPointF(15, 8)));

    bottomRightWorkspaceRect = QRectF(snapToNearestPixel(QPointF(10, 10)), snapToNearestPixel(QPointF(15, 14)));

    bottomLeftWorkspaceRect = QRectF(snapToNearestPixel(QPointF(3, 10)), snapToNearestPixel(QPointF(8, 14)));

    qreal workspaceWidth = topLeftWorkspaceRect.right() - topLeftWorkspaceRect.left();
    qreal workspaceHeight = topLeftWorkspaceRect.bottom() - topLeftWorkspaceRect.top();
    qreal workspaceSpacing =
        std::min(topRightWorkspaceRect.left() - topLeftWorkspaceRect.right(), bottomLeftWorkspaceRect.top() - topLeftWorkspaceRect.bottom());

    // make adjustments to correct snapping distortions
    topRightWorkspaceRect.setTopLeft(topLeftWorkspaceRect.topRight() + QPointF(workspaceSpacing, 0));
    topRightWorkspaceRect.setBottomRight(topLeftWorkspaceRect.bottomRight() + QPointF(workspaceSpacing + workspaceWidth, 0));

    bottomLeftWorkspaceRect.setTopLeft(topLeftWorkspaceRect.bottomLeft() + QPointF(0, workspaceSpacing));
    bottomLeftWorkspaceRect.setBottomRight(topLeftWorkspaceRect.bottomRight() + QPointF(0, workspaceSpacing + workspaceHeight));

    bottomRightWorkspaceRect.setTopLeft(topRightWorkspaceRect.bottomLeft() + QPointF(0, workspaceSpacing));
    bottomRightWorkspaceRect.setBottomRight(topRightWorkspaceRect.bottomRight() + QPointF(0, workspaceSpacing + workspaceHeight));

    topLeftWorkspace.addRect(topLeftWorkspaceRect);
    topRightWorkspace.addRect(topRightWorkspaceRect);
    bottomLeftWorkspace.addRect(bottomLeftWorkspaceRect);
    bottomRightWorkspace.addRect(bottomRightWorkspaceRect);

    std::unique_ptr<QGraphicsScene> desktops = std::unique_ptr<QGraphicsScene>(new QGraphicsScene(0, 0, 18, 18));
    QGraphicsItemGroup *desktopsGroup = new QGraphicsItemGroup;

    desktops->addItem(desktopsGroup);
    QGraphicsPathItem *topLeft = new QGraphicsPathItem;
    QGraphicsPathItem *topRight = new QGraphicsPathItem;
    QGraphicsPathItem *bottomLeft = new QGraphicsPathItem;
    QGraphicsPathItem *bottomRight = new QGraphicsPathItem;
    desktopsGroup->addToGroup(topLeft);
    desktopsGroup->addToGroup(topRight);
    desktopsGroup->addToGroup(bottomLeft);
    desktopsGroup->addToGroup(bottomRight);
    topLeft->setPen(pen);
    topLeft->setBrush(brush);
    topLeft->setPath(topLeftWorkspace);
    topRight->setPen(pen);
    topRight->setBrush(brush);
    topRight->setPath(topRightWorkspace);
    bottomLeft->setPen(pen);
    bottomLeft->setBrush(brush);
    bottomLeft->setPath(bottomLeftWorkspace);
    bottomRight->setPen(pen);
    bottomRight->setBrush(brush);
    bottomRight->setPath(bottomRightWorkspace);

    // centre -- centre the result, then snap centred position to a pixel boundary
    QPointF centerTranslate =
        QPointF(9, 9) - desktopsGroup->childrenBoundingRect().center(); // must use childrenBoundingRect as boundingRect does not update unless you add an item
    if (centerTranslate != QPointF(0, 0)) {
        QPointF centrePixelRealignmentOffset;

        // realign to pixel grid after centering -- use P1 of top to sample
        centrePixelRealignmentOffset =
            snapToNearestPixel(topLeft->path().elementAt(0) + centerTranslate, SnapPixel::ToWhole, SnapPixel::ToWhole) - topLeft->path().elementAt(0);
        QTransform transformToAlignedCenter;
        transformToAlignedCenter.translate(centrePixelRealignmentOffset.x(), centrePixelRealignmentOffset.y());
        desktopsGroup->setTransform(transformToAlignedCenter);
    }

    desktops->render(m_painter, QRectF(0, 0, 18, 18), QRectF(0, 0, 18, 18));
}

void RenderDecorationButtonIcon18By18::renderShadeIcon()
{
    QPen pen = m_painter->pen();
    if (m_totalScalingFactor < 1.3)
        pen.setCapStyle(Qt::FlatCap); // prevents lobsided arrow at loDPI

    bool isOddPenWidth = true;
    qreal roundedBoldPenWidth = 1;
    if (m_boldButtonIcons) {
        // thicker pen in titlebar
        isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, 1.3);
    } else {
        isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, 1);
    }
    pen.setWidthF(roundedBoldPenWidth);
    m_painter->setPen(pen);

    QVector<QPointF> line1, line2;

    if (isOddPenWidth) {
        line1 = {snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf),
                 snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf)};
        line2 = {QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)};
    } else {
        line1 = {snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                 snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole)};
        line2 = {QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)};
    }

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

void RenderDecorationButtonIcon18By18::renderUnShadeIcon()
{
    QPen pen = m_painter->pen();

    if (m_totalScalingFactor < 1.3)
        pen.setCapStyle(Qt::FlatCap); // prevents lobsided arrow at loDPI

    bool isOddPenWidth = true;
    qreal roundedBoldPenWidth = 1;
    if (m_boldButtonIcons) {
        // thicker pen in titlebar
        isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, 1.3);
    } else {
        isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, 1);
    }
    pen.setWidthF(roundedBoldPenWidth);
    m_painter->setPen(pen);

    QVector<QPointF> line1, line2;

    if (isOddPenWidth) {
        line1 = {snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf),
                 snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf)};
        line2 = {QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)};
    } else {
        line1 = {snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                 snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole)};
        line2 = {QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)};
    }

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

void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
{
    // a selected x

    // first determine the size of the maximize icon so the icon can align with it vertically
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);

    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * straightLineOpacity());
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPainterPath outerRing;
    outerRing.addEllipse(QRectF(3, 3, 12, 12));
    // centre
    QPointF centerTranslate = QPointF(9, maximizeRect.center().y()) - outerRing.boundingRect().center();
    outerRing.translate(centerTranslate);
    m_painter->drawPath(outerRing);

    QPainterPath innerX;

    // larger inner X touching the circle:
    qreal innerXSquareWidth = std::sqrt(std::pow(outerRing.boundingRect().width(), 2) / 2); // pythagoras
    QRectF innerXSquare(0, 0, innerXSquareWidth, innerXSquareWidth);
    centerTranslate = (outerRing.boundingRect().center() - innerXSquare.center());
    pen.setCapStyle(Qt::PenCapStyle::FlatCap);
    innerX.moveTo(innerXSquare.topLeft());
    innerX.lineTo(innerXSquare.bottomRight());
    innerX.moveTo(innerXSquare.topRight());
    innerX.lineTo(innerXSquare.bottomLeft());

    // Small inner X:
    // innerX.moveTo(7, 7);
    // innerX.lineTo(11, 11);
    // innerX.moveTo(11, 7);
    // innerX.lineTo(7, 11);

    innerX.translate(centerTranslate);
    pen = m_painter->pen();
    pen.setColor(color);
    m_painter->setPen(pen);
    m_painter->drawPath(innerX);
}

void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
{
    // a selected dot

    // first determine the size of the maximize icon so the icon can align with it vertically
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    QPen pen = m_painter->pen();
    pen.setWidthF(maximizePenWidth);

    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * straightLineOpacity());
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPainterPath outerRing;
    outerRing.addEllipse(QRectF(3, 3, 12, 12));
    // centre
    QPointF centerTranslate = QPointF(9, maximizeRect.center().y()) - outerRing.boundingRect().center();
    outerRing.translate(centerTranslate);
    m_painter->drawPath(outerRing);

    QPainterPath innerDot;
    innerDot.addEllipse(QRectF(7.5, 7.5, 3, 3));
    innerDot.translate(centerTranslate);
    m_painter->setBrush(color);
    m_painter->setPen(Qt::NoPen);
    m_painter->drawPath(innerDot);
}

void RenderDecorationButtonIcon18By18::renderApplicationMenuIcon()
{
    bool isOddPenWidth = true;

    QPen pen = m_painter->pen();
    pen.setCapStyle(Qt::PenCapStyle::FlatCap);

    qreal roundedBoldPenWidth = 1;
    qreal boldingFactor = m_boldButtonIcons ? m_squareMaximizeBoldPenWidthFactor : 1;
    isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, boldingFactor);

    pen.setWidthF(roundedBoldPenWidth);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    std::unique_ptr<QGraphicsScene> appMenu = std::unique_ptr<QGraphicsScene>(new QGraphicsScene(0, 0, 18, 18));

    QGraphicsItemGroup *appMenuGroup = new QGraphicsItemGroup;

    appMenu->addItem(appMenuGroup);
    QGraphicsPathItem *top = new QGraphicsPathItem;
    QGraphicsPathItem *middle = new QGraphicsPathItem;
    QGraphicsPathItem *bottom = new QGraphicsPathItem;
    appMenuGroup->addToGroup(top);
    appMenuGroup->addToGroup(middle);
    appMenuGroup->addToGroup(bottom);

    if (isOddPenWidth) {
        QPointF leftTop = snapToNearestPixel(QPointF(3.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf);
        QPointF leftMiddle = snapToNearestPixel(QPointF(3.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToHalf);
        QPointF leftBottom = snapToNearestPixel(QPointF(3.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToHalf);
        QPointF rightTop = snapToNearestPixel(QPointF(14.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf);

        qreal verticalSpacing = std::max(leftMiddle.y() - leftTop.y(), leftBottom.y() - leftMiddle.y());
        leftMiddle.setY(leftTop.y() + verticalSpacing);
        leftBottom.setY(leftMiddle.y() + verticalSpacing);

        qreal width = rightTop.x() - leftTop.x();

        QPainterPath topPath;
        topPath.addPolygon(QVector<QPointF>{leftTop, rightTop});
        top->setPath(topPath);
        QPainterPath middlePath;
        middlePath.addPolygon(QVector<QPointF>{leftMiddle, QPointF(leftMiddle.x() + width, leftMiddle.y())});
        middle->setPath(middlePath);
        QPainterPath bottomPath;
        bottomPath.addPolygon(QVector<QPointF>{leftBottom, QPointF(leftBottom.x() + width, leftBottom.y())});
        bottom->setPath(bottomPath);
    } else {
        QPointF leftTop = snapToNearestPixel(QPointF(3.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF leftMiddle = snapToNearestPixel(QPointF(3.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF leftBottom = snapToNearestPixel(QPointF(3.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF rightTop = snapToNearestPixel(QPointF(14.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole);

        qreal verticalSpacing = std::max(leftMiddle.y() - leftTop.y(), leftBottom.y() - leftMiddle.y());
        leftMiddle.setY(leftTop.y() + verticalSpacing);
        leftBottom.setY(leftMiddle.y() + verticalSpacing);

        qreal width = rightTop.x() - leftTop.x();

        QPainterPath topPath;
        topPath.addPolygon(QVector<QPointF>{leftTop, rightTop});
        top->setPath(topPath);
        QPainterPath middlePath;
        middlePath.addPolygon(QVector<QPointF>{leftMiddle, QPointF(leftMiddle.x() + width, leftMiddle.y())});
        middle->setPath(middlePath);
        QPainterPath bottomPath;
        bottomPath.addPolygon(QVector<QPointF>{leftBottom, QPointF(leftBottom.x() + width, leftBottom.y())});
        bottom->setPath(bottomPath);
    }

    top->setPen(pen);
    middle->setPen(pen);
    bottom->setPen(pen);

    // centre -- centre the result, then snap centred position to a pixel boundary
    QPointF centerTranslate =
        QPointF(9, 9) - appMenuGroup->childrenBoundingRect().center(); // must use childrenBoundingRect as boundingRect does not update unless you add an item
    if (centerTranslate != QPointF(0, 0)) {
        QPointF centrePixelRealignmentOffset;

        // realign to pixel grid after centering -- use P1 of top to sample
        centrePixelRealignmentOffset =
            snapToNearestPixel(top->path().elementAt(0) + centerTranslate, SnapPixel::ToWhole, isOddPenWidth ? SnapPixel::ToHalf : SnapPixel::ToWhole)
            - top->path().elementAt(0);
        QTransform transformToAlignedCenter;
        transformToAlignedCenter.translate(centrePixelRealignmentOffset.x(), centrePixelRealignmentOffset.y());
        appMenuGroup->setTransform(transformToAlignedCenter);
    }

    if (m_strokeToFilledPath) {
        QPainterPathStroker stroker(pen);
        top->setPath(stroker.createStroke(top->path()));
        middle->setPath(stroker.createStroke(middle->path()));
        bottom->setPath(stroker.createStroke(bottom->path()));

        top->setBrush(pen.color());
        middle->setBrush(pen.color());
        bottom->setBrush(pen.color());

        top->setPen(Qt::NoPen);
        middle->setPen(Qt::NoPen);
        bottom->setPen(Qt::NoPen);
    }

    appMenu->render(m_painter, QRectF(0, 0, 18, 18), QRectF(0, 0, 18, 18));
}

void RenderDecorationButtonIcon18By18::renderCloseIconAtSquareMaximizeSize()
{
    // first determine the size of the maximize icon so the close icon can match in size
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);
    QPen pen = m_painter->pen();

    pen.setWidthF(maximizePenWidth);

    m_painter->setPen(pen);
    if (m_shrinkCloseIcon) { // render smaller close icon with thicker width from here
        RenderDecorationButtonIcon18By18::renderCloseIcon();
        return;
    }

    QVector<QPointF> line1{maximizeRect.topLeft(), maximizeRect.bottomRight()};
    QVector<QPointF> line2{maximizeRect.topRight(), maximizeRect.bottomLeft()};

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

std::pair<QRectF, qreal> RenderDecorationButtonIcon18By18::renderSquareMaximizeIcon(bool returnSizeOnly, qreal cornerRelativePercent)
{
    if (returnSizeOnly)
        m_painter->save(); // needed so doesn't interfere when called from renderCloseIconAtSquareMaximizeSize etc.

    QPen pen = m_painter->pen();

    bool isOddPenWidth = true;
    qreal roundedBoldPenWidth = 1;
    if (m_boldButtonIcons) {
        isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, m_squareMaximizeBoldPenWidthFactor);
    } else {
        isOddPenWidth = roundedPenWidthIsOdd(pen, roundedBoldPenWidth, 1);
    }
    pen.setWidthF(roundedBoldPenWidth);

    QRectF rect(QPointF(4.5, 4.5), QPointF(13.5, 13.5));
    qreal adjustmentOffset = 0;
    qreal halfDevicePixelInLocal = convertDevicePixelsToLocal(0.5);
    constexpr int maxIterations = 5;
    int i = 0;
    qreal designedWidthDevicePixels = 9 * m_totalScalingFactor;
    do {
        if (isOddPenWidth) {
            rect = QRectF(snapToNearestPixel(rect.topLeft(), SnapPixel::ToHalf, SnapPixel::ToHalf),
                          snapToNearestPixel(rect.bottomRight(), SnapPixel::ToHalf, SnapPixel::ToHalf));
        } else {
            rect = QRectF(snapToNearestPixel(rect.topLeft(), SnapPixel::ToWhole, SnapPixel::ToWhole),
                          snapToNearestPixel(rect.bottomRight(), SnapPixel::ToWhole, SnapPixel::ToWhole));
        }

        qreal maxSide =
            qMax(rect.width(), rect.height()); // ensure width and height are the same -- they are sometimes not on display scales which are not a factor of 0.5
        rect.setTopLeft(QPointF(0, 0));
        rect.setBottomRight(QPointF(maxSide, maxSide));

        qreal rectWidthDevicePixels = rect.width() * m_totalScalingFactor;
        bool increaseSize = false;
        if (m_forceEvenSquares) {
            // if m_forceEvenSquares and total device square width is not even
            if (qRound(rectWidthDevicePixels + penWidthToDevice(pen)) % 2 != 0) {
                increaseSize = true;
            }
        } else {
            // if size is still smaller than linear to original design, increase again
            if (rectWidthDevicePixels < (designedWidthDevicePixels - 0.001)) { // 0.001 as sometimes there are floating point errors
                increaseSize = true;
            }
        }

        if (increaseSize) {
            adjustmentOffset = halfDevicePixelInLocal;
            rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
        } else {
            adjustmentOffset = 0;
        }
        i++;
    } while (adjustmentOffset && i < maxIterations);

    // centre
    QPointF centerTranslate = QPointF(9, 9) - rect.center();
    if (centerTranslate != QPointF(0, 0)) {
        QPointF centrePixelRealignmentOffset;

        // realign to pixel grid after centring -- use top-left of rect to sample
        if (isOddPenWidth) {
            centrePixelRealignmentOffset = snapToNearestPixel(rect.topLeft() + centerTranslate, SnapPixel::ToHalf, SnapPixel::ToHalf) - rect.topLeft();
        } else {
            centrePixelRealignmentOffset = snapToNearestPixel(rect.topLeft() + centerTranslate, SnapPixel::ToWhole, SnapPixel::ToWhole) - rect.topLeft();
        }

        rect.moveTopLeft(rect.topLeft() + centrePixelRealignmentOffset);
    }

    if (!returnSizeOnly) {
        // make excessively thick pen widths translucent to balance with other buttons

        qreal opacity = straightLineOpacity();
        QColor penColor = pen.color();
        penColor.setAlphaF(penColor.alphaF() * opacity);
        pen.setColor(penColor);

        // convert strokes to paths needed as otherwise GTK apps fill the square in generated icons
        if (m_strokeToFilledPath) {
            QPainterPath path;
            path.addRoundedRect(rect, cornerRelativePercent, cornerRelativePercent, Qt::RelativeSize);
            QPainterPathStroker stroker(pen);
            path = stroker.createStroke(path);
            m_painter->setBrush(pen.color());
            m_painter->setPen(Qt::NoPen);
            m_painter->drawPath(path);
        } else {
            m_painter->setPen(pen);
            m_painter->drawRoundedRect(rect, cornerRelativePercent, cornerRelativePercent, Qt::RelativeSize);
        }
    } else
        m_painter->restore();

    return {rect, pen.widthF()};
}

void RenderDecorationButtonIcon18By18::renderTinySquareMinimizeIcon(bool showArrows)
{
    // first determine the size of the maximize icon so the minimize icon can align with it
    auto [maximizeRect, maximizePenWidth] = renderSquareMaximizeIcon(true);

    bool isOddPenWidth = true;

    QPen pen = m_painter->pen();
    qreal origOpacity = pen.color().alphaF();
    if (m_boldButtonIcons) {
        QColor penColor = pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.45);
        penColor.setAlphaF(penColor.alphaF() * straightLineOpacity() * 0.95);
        pen.setColor(penColor);

        m_painter->setBrush(brushColor);

        isOddPenWidth = qRound(maximizePenWidth) % 2 != 0;
        pen.setWidthF(maximizePenWidth);

        m_painter->setPen(pen);
    } else { // in fine mode the dense minimize button appears bolder than the others so reduce its opacity to compensate
        QColor penColor = pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.35);
        penColor.setAlphaF(penColor.alphaF() * straightLineOpacity() * 0.9);
        pen.setColor(penColor);

        m_painter->setBrush(brushColor);

        isOddPenWidth = qRound(maximizePenWidth) % 2 != 0;
        pen.setWidthF(maximizePenWidth);
        m_painter->setPen(pen);
    }

    // tiny filled square
    QRectF rect(QPointF(7.5, 7.5), QPointF(10.5, 10.5));
    qreal adjustmentOffset = 0;
    constexpr int maxIterations = 5;
    int i = 0;
    qreal designedWidthDevicePixels = 3 * m_totalScalingFactor;
    qreal halfDevicePixelInLocal = convertDevicePixelsToLocal(0.5);
    do {
        if (isOddPenWidth) {
            rect = QRectF(snapToNearestPixel(rect.topLeft(), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down),
                          snapToNearestPixel(rect.bottomRight(), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down));
        } else {
            rect = QRectF(snapToNearestPixel(rect.topLeft(), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down),
                          snapToNearestPixel(rect.bottomRight(), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down));
        }
        qreal maxSide =
            qMax(rect.width(), rect.height()); // ensure width and height are the same -- they are sometimes not on display scales which are not a factor of 0.5
        rect.setTopLeft(QPointF(0, 0));
        rect.setBottomRight(QPointF(maxSide, maxSide));

        qreal rectWidthDevicePixels = rect.width() * m_totalScalingFactor;
        bool increaseSize = false;
        if (m_forceEvenSquares) {
            // if m_forceEvenSquares and total device square width is not even
            if (qRound(rectWidthDevicePixels + penWidthToDevice(pen)) % 2 != 0) {
                increaseSize = true;
            }
        } else {
            // if size is still smaller than linear to original design, increase again
            if (rectWidthDevicePixels < (designedWidthDevicePixels - 0.001)) { // 0.001 as sometimes there are floating point errors
                increaseSize = true;
            }
        }

        if (increaseSize) {
            adjustmentOffset = halfDevicePixelInLocal;
            rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
        } else {
            adjustmentOffset = 0;
        }
        i++;
    } while (adjustmentOffset && i < maxIterations);

    // centre
    QPointF centerTranslate = QPointF(9, maximizeRect.center().y()) - rect.center();
    if (centerTranslate != QPointF(0, 0)) {
        QPointF centrePixelRealignmentOffset;

        // realign to pixel grid after centring -- use top-left of rect to sample
        if (isOddPenWidth) {
            centrePixelRealignmentOffset = snapToNearestPixel(rect.topLeft() + centerTranslate, SnapPixel::ToHalf, SnapPixel::ToHalf) - rect.topLeft();
        } else {
            centrePixelRealignmentOffset = snapToNearestPixel(rect.topLeft() + centerTranslate, SnapPixel::ToWhole, SnapPixel::ToWhole) - rect.topLeft();
        }
        m_painter->translate(centrePixelRealignmentOffset);
    }

    if (m_strokeToFilledPath) {
        m_painter->setPen(Qt::NoPen);
        m_painter->drawRect(rect);
        QPainterPath stroke;
        stroke.addRect(rect);
        QPainterPathStroker stroker(pen);
        stroke = stroker.createStroke(stroke);
        m_painter->setBrush(pen.color());
        m_painter->drawPath(stroke);
    } else {
        m_painter->drawRect(rect);
    }

    if (showArrows) {
        qreal penWidth18By18 = penWidthToLocal(pen);
        qreal halfPenWidth18By18 = penWidthToLocal(pen) / 2;
        // qreal arrowLength = rect.height() * 2 / 3 + penWidth18By18;
        qreal arrowLength = rect.height() + penWidth18By18;
        QPolygonF topArrow, bottomArrow;
        QPointF topArrowIntersection, bottomArrowIntersection;

        topArrowIntersection = QPointF(rect.topRight() - QPointF(halfPenWidth18By18, -halfPenWidth18By18));
        topArrow << topArrowIntersection << (topArrowIntersection - QPointF(0, arrowLength)) << (topArrowIntersection + QPointF(arrowLength, 0))
                 << topArrowIntersection;

        bottomArrowIntersection = QPointF(rect.bottomLeft() + QPointF(halfPenWidth18By18, -halfPenWidth18By18));
        bottomArrow << bottomArrowIntersection << (bottomArrowIntersection - QPointF(arrowLength, 0)) << (bottomArrowIntersection + QPointF(0, arrowLength))
                    << bottomArrowIntersection;

        /*
        topArrowIntersection = QPointF(rect.topLeft());
        topArrow << topArrowIntersection << (topArrowIntersection - QPointF(arrowLength, 0)) << (topArrowIntersection - QPointF(0, arrowLength))
                 << topArrowIntersection;

        bottomArrowIntersection = QPointF(rect.bottomRight());
        bottomArrow << bottomArrowIntersection << (bottomArrowIntersection + QPointF(arrowLength, 0)) << (bottomArrowIntersection + QPointF(0, arrowLength))
                    << bottomArrowIntersection;
        */

        QColor penColor = pen.color();
        penColor.setAlphaF(origOpacity);
        m_painter->setBrush(penColor);
        m_painter->setPen(Qt::NoPen);
        m_painter->drawConvexPolygon(topArrow);
        m_painter->drawConvexPolygon(bottomArrow);
    }
}

void RenderDecorationButtonIcon18By18::renderContextHelpIcon()
{
    QPen pen = m_painter->pen();

    if (m_boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.6);
    }

    pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(pen);

    // main body of question mark
    QPainterPath path;
    path.arcMoveTo(QRectF(6.5, 3.5, 5.5, 5), 150);
    path.arcTo(QRectF(6.5, 3.5, 5.5, 5), 150, -150);
    path.cubicTo(QPointF(12, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    if (m_strokeToFilledPath) {
        QPainterPathStroker stroker(m_painter->pen());
        path = stroker.createStroke(path);
        ;
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(pen.color());
    }
    m_painter->drawPath(path);

    // dot of question mark
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(pen.color());
    if (m_boldButtonIcons)
        m_painter->drawEllipse(QRectF(8, 14, 2, 2));
    else
        m_painter->drawEllipse(QRectF(8.25, 14.25, 1.5, 1.5));
}

void RenderDecorationButtonIcon18By18::renderExcludeFromCaptureIcon()
{
    QPen pen = m_painter->pen();
    if (m_boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.6);
    }

    pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(pen);

    QPainterPath path;

    // Fedora brim
    path.moveTo(1, 8);
    path.lineTo(17, 8);

    // Fedora crown with center crease
    path.moveTo(3.5, 7.5);
    path.quadTo(3.5, 2.5, 6.5, 2.5);
    path.cubicTo(7, 2.5, 8, 3.5, 9, 4);
    path.cubicTo(10, 3.5, 11, 2.5, 12, 2.5);
    path.quadTo(14.5, 2.5, 14.5, 7.5);
    path.closeSubpath();

    // Glasses
    path.addEllipse(QRectF(2.5, 9.5, 5.5, 5.5));
    path.addEllipse(QRectF(10, 9.5, 5.5, 5.5));
    path.moveTo(8, 11.5);
    path.lineTo(10, 11.5);

    if (m_strokeToFilledPath) {
        QPainterPathStroker stroker(m_painter->pen());
        path = stroker.createStroke(path);

        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(pen.color());
    }
    m_painter->drawPath(path);
}
}
