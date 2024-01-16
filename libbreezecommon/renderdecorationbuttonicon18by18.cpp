/*
 * SPDX-FileCopyrightText: 2022-2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon18by18.h"
#include <QGraphicsItem>

namespace Breeze
{

RenderDecorationButtonIcon18By18::RenderDecorationButtonIcon18By18(QPainter *painter,
                                                                   const bool fromKstyle,
                                                                   const bool boldButtonIcons,
                                                                   const qreal devicePixelRatio,
                                                                   const QPointF &deviceOffsetFromZeroReference)
    : RenderDecorationButtonIcon(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetFromZeroReference)
{
}

/* most base methods here are KDE's default Breeze/Oxygen style -- override with other styles */
void RenderDecorationButtonIcon18By18::renderCloseIcon()
{
    m_painter->drawLine(QPointF(5, 5), QPointF(13, 13));
    m_painter->drawLine(QPointF(13, 5), QPointF(5, 13));
}

void RenderDecorationButtonIcon18By18::renderMaximizeIcon()
{
    // up arrow
    m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)});
}

void RenderDecorationButtonIcon18By18::renderRestoreIcon()
{
    QPen pen = m_painter->pen();
    pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(pen);

    if (m_fromKstyle) { // slightly smaller diamond

        // diamond / floating kite
        m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)});

    } else {
        // diamond / floating kite
        m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9), QPointF(9, 14)});
    }
}

void RenderDecorationButtonIcon18By18::renderMinimizeIcon()
{
    // down arrow
    m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)});
}

void RenderDecorationButtonIcon18By18::renderPinnedOnAllDesktopsIcon()
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

void RenderDecorationButtonIcon18By18::renderPinOnAllDesktopsIcon()
{
    QPen pen = m_painter->pen();
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    m_painter->setBrush(color);
    m_painter->setPen(Qt::NoPen);
    m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(6.5, 8.5), QPointF(12, 3), QPointF(15, 6), QPointF(9.5, 11.5)});

    pen.setColor(color);
    m_painter->setPen(pen);
    m_painter->drawLine(QPointF(5.5, 7.5), QPointF(10.5, 12.5));
    m_painter->drawLine(QPointF(8, 10), QPointF(4.5, 13.5));
}

void RenderDecorationButtonIcon18By18::renderShadeIcon()
{
    QPen pen = m_painter->pen();
    if (m_totalScalingFactor < 1.3)
        pen.setCapStyle(Qt::FlatCap); // prevents lobsided arrow at loDPI

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.3);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(pen);
    }

    if (isOddPenWidth) {
        m_painter->drawLine(snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf),
                            snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf));
        m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
    } else {
        m_painter->drawLine(snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                            snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole));
        m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
    }
}

void RenderDecorationButtonIcon18By18::renderUnShadeIcon()
{
    QPen pen = m_painter->pen();

    if (m_totalScalingFactor < 1.3)
        pen.setCapStyle(Qt::FlatCap); // prevents lobsided arrow at loDPI

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.3);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(pen);
    }

    if (isOddPenWidth) {
        m_painter->drawLine(snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf),
                            snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToHalf));
        m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)});
    } else {
        m_painter->drawLine(snapToNearestPixel(QPointF(4, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                            snapToNearestPixel(QPointF(14, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole));
        m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)});
    }
}

void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
{
    // two down arrows
    m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 5), QPointF(9, 10), QPointF(14, 5)});

    m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 14), QPointF(14, 9)});
}

void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
{
    // two up arrows
    m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9)});

    m_painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
}

void RenderDecorationButtonIcon18By18::renderApplicationMenuIcon()
{
    bool isOddPenWidth = true;

    QPen pen = m_painter->pen();
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(pen);
    }

    m_painter->setBrush(pen.color());

    if (isOddPenWidth) {
        QPointF topLeftTop = snapToNearestPixel(QPointF(3.5, 4.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF topLeftMiddle = snapToNearestPixel(QPointF(3.5, 8.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF topLeftBottom = snapToNearestPixel(QPointF(3.5, 12.5), SnapPixel::ToHalf, SnapPixel::ToHalf);

        qreal verticalSpacing = std::max(topLeftMiddle.y() - topLeftTop.y(), topLeftBottom.y() - topLeftMiddle.y());
        topLeftMiddle.setY(topLeftTop.y() + verticalSpacing);
        topLeftBottom.setY(topLeftMiddle.y() + verticalSpacing);

        m_painter->drawRect(QRectF(topLeftTop, QSizeF(11, 1)));
        m_painter->drawRect(QRectF(topLeftMiddle, QSizeF(11, 1)));
        m_painter->drawRect(QRectF(topLeftBottom, QSizeF(11, 1)));
    } else {
        QPointF topLeftTop = snapToNearestPixel(QPointF(3.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF topLeftMiddle = snapToNearestPixel(QPointF(3.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF topLeftBottom = snapToNearestPixel(QPointF(3.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToWhole);

        qreal verticalSpacing = std::max(topLeftMiddle.y() - topLeftTop.y(), topLeftBottom.y() - topLeftMiddle.y());
        topLeftMiddle.setY(topLeftTop.y() + verticalSpacing);
        topLeftBottom.setY(topLeftMiddle.y() + verticalSpacing);

        m_painter->drawRect(QRectF(topLeftTop, QSizeF(11, 1)));
        m_painter->drawRect(QRectF(topLeftMiddle, QSizeF(11, 1)));
        m_painter->drawRect(QRectF(topLeftBottom, QSizeF(11, 1)));
    }
}

void RenderDecorationButtonIcon18By18::renderContextHelpIcon()
{
    QPainterPath path;
    path.moveTo(5, 6);
    path.arcTo(QRectF(5, 3.5, 8, 5), 180, -180);
    path.cubicTo(QPointF(12.5, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    m_painter->drawPath(path);

    m_painter->drawRect(QRectF(9, 15, 0.5, 0.5));
}

void RenderDecorationButtonIcon18By18::renderCloseIconAtSquareMaximizeSize()
{
    // first determine the size of the maximize icon so the close icon can match in size
    qreal maximizeSize = renderSquareMaximizeIcon(true);
    QPen pen = m_painter->pen();

    if (m_fromKstyle) {
        pen.setWidthF(pen.widthF() * 1.2);
    } else if (m_boldButtonIcons) {
        pen.setWidthF(qRound(pen.widthF() * 1.5));
    }

    m_painter->setPen(pen);

    QPointF topLeft;
    topLeft.setX((18 - maximizeSize) / 2);
    topLeft.setY(topLeft.x());

    m_painter->drawLine(topLeft, QPointF(topLeft.x() + maximizeSize, topLeft.y() + maximizeSize)); // top-left to bottom-right
    m_painter->drawLine(QPointF(topLeft.x() + maximizeSize, topLeft.y()), QPointF(topLeft.x(), topLeft.y() + maximizeSize)); // top-right to bottom-left
}

qreal RenderDecorationButtonIcon18By18::renderSquareMaximizeIcon(bool returnSizeOnly)
{
    if (returnSizeOnly)
        m_painter->save(); // needed so doesn't interfere when called from renderCloseIcon

    QPen pen = m_painter->pen();

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_squareMaximizeBoldPenWidthFactor);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth);
    }

    QRectF rect(snapToNearestPixel(QPointF(4.5, 4.5), SnapPixel::ToHalf, SnapPixel::ToHalf),
                snapToNearestPixel(QPointF(13.5, 13.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down));

    qreal width = rect.width();
    qreal height = rect.height();
    if (width != height) { // happens on display scales which are not a factor of 0.5
        qreal maxSide = std::max(width, height);
        rect.setBottomRight(QPointF(rect.topLeft().x() + maxSide, rect.topLeft().y() + maxSide));
    }

    if (!isOddPenWidth) { // enlarge the rectangle if even to snap to whole pixels
        qreal adjustmentOffset = convertDevicePixelsToLocal(0.5);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    // if size is still smaller than linear to original design, increase again
    if ((rect.width() * m_totalScalingFactor) < (9 * m_totalScalingFactor - 0.0001)) { // 0.0001 as sometimes there are floating point errors
        qreal adjustmentOffset = convertDevicePixelsToLocal(1);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    if (!returnSizeOnly) {
        // make excessively thick pen widths translucent to balance with other buttons

        qreal opacity = straightLineOpacity();
        QColor penColor = pen.color();
        penColor.setAlphaF(penColor.alphaF() * opacity);
        pen.setColor(penColor);

        m_painter->setPen(pen);
        m_painter->drawRoundedRect(rect, 0.025, 0.025, Qt::RelativeSize);
    } else
        m_painter->restore();

    return rect.height();
}

void RenderDecorationButtonIcon18By18::renderOverlappingWindowsIcon()
{
    QPen pen = m_painter->pen();

    pen.setJoinStyle(Qt::BevelJoin);
    pen.setCapStyle(Qt::FlatCap);

    int roundedBoldPenWidth = 1;
    bool isOddPenWidth = true;
    if (m_boldButtonIcons) {
        isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_overlappingWindowsBoldPenWidthFactor);
    } else {
        isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
    }

    // thicker pen in titlebar
    pen.setWidthF(roundedBoldPenWidth);

    // this is to calculate the offset to move the two rectangles further from each other onto an aligned pixel
    // they can be moved apart as the line thickness increases -- this prevents blurriness when the lines are drawn too close together
    qreal shiftOffset = 0;

    QGraphicsItemGroup *overlappingWindowsGroup = nullptr;
    QGraphicsRectItem *foregroundRect = nullptr;
    QGraphicsPathItem *backgroundPath = nullptr;

    std::unique_ptr<QGraphicsScene> overlappingWindows =
        calculateOverlappingWindowsScene(isOddPenWidth, shiftOffset, overlappingWindowsGroup, foregroundRect, backgroundPath);

    if (!(overlappingWindows && overlappingWindowsGroup && foregroundRect && backgroundPath))
        return;

    qreal distanceBetweenSquares = std::min(backgroundPath->path().elementAt(3).x - backgroundPath->path().elementAt(4).x,
                                            backgroundPath->path().elementAt(0).y - backgroundPath->path().elementAt(1).y);
    qreal penWidth18By18 = penWidthToLocal(pen);

    // if distance between squares < pen width (factoring in that the background sqaure does not join the foreground at the foreground's centre-point)
    // || distance between squares < 2
    if (((distanceBetweenSquares / penWidth18By18) < 1.25 - 0.0001) || (m_boldButtonIcons && distanceBetweenSquares < 2 - 0.0001)
        || (!m_boldButtonIcons && distanceBetweenSquares < 1.5 - 0.0001) // 0.0001 is because sometimes there are floating point errors
    ) {
        // generate it again using a larger shiftOffset to push apart the squares
        overlappingWindows.reset();
        overlappingWindowsGroup = nullptr;
        foregroundRect = nullptr;
        backgroundPath = nullptr;
        overlappingWindows = calculateOverlappingWindowsScene(isOddPenWidth,
                                                              shiftOffset + convertDevicePixelsToLocal(1),
                                                              overlappingWindowsGroup,
                                                              foregroundRect,
                                                              backgroundPath);
        if (!(overlappingWindows && overlappingWindowsGroup && foregroundRect && backgroundPath))
            return;
    }

    // the following comment block is for centring the result, and then snapping the centred position to a pixel boundary -- no longer deemed needed
    /*
     // centre -- the generated group is not always centred
    QPointF centerTranslate = QPointF(9,9) - overlappingWindowsGroup->boundingRect().center();

    qreal centrePixelRealignmentOffsetX;
    qreal centrePixelRealignmentOffsetY;

    //realign to pixel grid after centering -- use top-left of foregroundRect to sample
    if (isOddPenWidth) {
        centrePixelRealignmentOffsetX = snapToNearestHalfPixel(foregroundRect->boundingRect().topLeft().x() + centerTranslate.x(), true) -
    foregroundRect->boundingRect().topLeft().x(); centrePixelRealignmentOffsetY = snapToNearestHalfPixel(foregroundRect->boundingRect().topLeft().y() +
    centerTranslate.y(), true) - foregroundRect->boundingRect().topLeft().y(); } else { centrePixelRealignmentOffsetX =
    snapToNearestWholePixel(foregroundRect->boundingRect().topLeft().x() + centerTranslate.x(), true) - foregroundRect->boundingRect().topLeft().x();
        centrePixelRealignmentOffsetY = snapToNearestWholePixel(foregroundRect->boundingRect().topLeft().y() + centerTranslate.y(), true) -
    foregroundRect->boundingRect().topLeft().y();
    }


    QTransform transformToAlignedCenter;
    transformToAlignedCenter.translate(centrePixelRealignmentOffsetX,centrePixelRealignmentOffsetY);
    overlappingWindowsGroup->setTransform(transformToAlignedCenter);

    */
    // make excessively thick pen widths translucent to balance with other buttons
    qreal opacity = straightLineOpacity();
    QColor penColor = pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    pen.setColor(penColor);
    // set the pen widths in all items -- do this at this point as not to have non-cosmetic pen widths in boundingRect().width() calculations above
    foregroundRect->setPen(pen);
    backgroundPath->setPen(pen);

    // paint
    overlappingWindows->render(m_painter, QRectF(0, 0, 18, 18), QRectF(0, 0, 18, 18));
}

std::unique_ptr<QGraphicsScene> RenderDecorationButtonIcon18By18::calculateOverlappingWindowsScene(const bool isOddPenWidth,
                                                                                                   const qreal shiftOffset,
                                                                                                   QGraphicsItemGroup *&overlappingWindowsGroup,
                                                                                                   QGraphicsRectItem *&foregroundSquareItem,
                                                                                                   QGraphicsPathItem *&backgroundSquareItem)
{
    std::unique_ptr<QGraphicsScene> overlappingWindows = std::unique_ptr<QGraphicsScene>(new QGraphicsScene(0, 0, 18, 18));

    overlappingWindowsGroup = new QGraphicsItemGroup;

    overlappingWindows->addItem(overlappingWindowsGroup);

    // overlapping windows icon
    // foreground square
    QPointF topLeft{4.5, 6.5};
    if (isOddPenWidth)
        topLeft = snapToNearestPixel(topLeft, SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
    else
        topLeft = snapToNearestPixel(topLeft, SnapPixel::ToWhole, SnapPixel::ToWhole);

    QPointF bottomRight{11.5, 13.5};
    if (isOddPenWidth)
        bottomRight = snapToNearestPixel(bottomRight, SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
    else
        bottomRight = snapToNearestPixel(bottomRight, SnapPixel::ToWhole, SnapPixel::ToWhole);

    qreal diameter = qMax((bottomRight.ry() - topLeft.y()), (bottomRight.x() - topLeft.x()));

    QRectF foregroundSquare(QPointF(bottomRight.x() - diameter, topLeft.y()), QPointF(bottomRight.x(), topLeft.y() + diameter));
    foregroundSquare.adjust(-shiftOffset, shiftOffset, -shiftOffset, shiftOffset);
    diameter = foregroundSquare.width();

    foregroundSquareItem = new QGraphicsRectItem(foregroundSquare);

    // background square
    QVector<QPointF> background{QPointF(6.5, 6), QPointF(6.5, 4.5), QPointF(13.5, 4.5), QPointF(13.5, 11.5), QPointF(12, 11.5)};

    if (isOddPenWidth)
        background[1] = snapToNearestPixel(background[1], SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Up, ThresholdRound::Down);
    else
        background[1] = snapToNearestPixel(background[1], SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Up, ThresholdRound::Down);

    background[0].setX(background[1].x());
    background[0].setY(foregroundSquare.top() - convertDevicePixelsToLocal(0.5));

    background[2].setX(background[1].x() + diameter);
    background[2].setY(background[1].y());

    background[3].setX(background[2].x());
    background[3].setY(background[2].y() + diameter);

    background[4].setX(foregroundSquare.right() + convertDevicePixelsToLocal(0.5));
    background[4].setY(background[3].y());

    QPainterPath backgroundSquarePath;
    backgroundSquarePath.addPolygon(background);
    backgroundSquareItem = new QGraphicsPathItem(backgroundSquarePath);

    overlappingWindowsGroup->addToGroup(foregroundSquareItem);
    overlappingWindowsGroup->addToGroup(backgroundSquareItem);

    // set no pen to make all dimension calculations simpler
    foregroundSquareItem->setPen(Qt::PenStyle::NoPen);
    backgroundSquareItem->setPen(Qt::PenStyle::NoPen);

    return overlappingWindows;
}

void RenderDecorationButtonIcon18By18::renderTinySquareMinimizeIcon()
{
    bool isOddPenWidth = true;
    int roundedBoldPenWidth;

    QPen pen = m_painter->pen();
    if (m_boldButtonIcons) {
        QColor penColor = pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.7);
        penColor.setAlphaF(penColor.alphaF() * 0.9);
        pen.setColor(penColor);

        pen.setJoinStyle(Qt::BevelJoin);
        m_painter->setBrush(brushColor);

        isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth);

        m_painter->setPen(pen);
    } else { // in fine mode the dense minimize button appears bolder than the others so reduce its opacity to compensate
        QColor penColor = pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.55);
        penColor.setAlphaF(penColor.alphaF() * 0.75);
        pen.setColor(penColor);

        pen.setJoinStyle(Qt::BevelJoin);
        m_painter->setBrush(brushColor);

        isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(pen);
    }

    // tiny filled square
    QRectF rect(snapToNearestPixel(QPointF(7.5, 7.5), SnapPixel::ToHalf, SnapPixel::ToHalf),
                snapToNearestPixel(QPointF(10.5, 10.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down));
    qreal width = rect.width();
    qreal height = rect.height();
    if (width != height) { // happens on display scales which are not a factor of 0.5
        qreal maxSide = std::max(width, height);
        rect.setBottomRight(QPointF(rect.topLeft().x() + maxSide, rect.topLeft().y() + maxSide));
    }

    if (!isOddPenWidth) { // enlarge the rectangle if even to snap to whole pixels
        qreal adjustmentOffset = convertDevicePixelsToLocal(0.5);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    /*
    // if size is still smaller than linear to original design, increase again
    if ((rect.width() * m_totalScalingFactor) < (3 * m_totalScalingFactor - 0.0001)) { // 0.0001 as sometimes there are floating point errors
        qreal adjustmentOffset = convertDevicePixelsToLocal(1);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }
    */

    m_painter->drawRect(rect);
}

// For consistency with breeze icon set
void RenderDecorationButtonIcon18By18::renderKeepBehindIconAsFromBreezeIcons()
{
    QPen pen = m_painter->pen();

    bool flatCapArrowHead = false;
    if (m_totalScalingFactor < 1.3) {
        pen.setCapStyle(Qt::FlatCap); // prevents blurry lobsided arrow at loDPI
        flatCapArrowHead = true;
    }

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.2);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        pen.setWidthF(roundedBoldPenWidth);
    }
    m_painter->setPen(pen);

    if (isOddPenWidth) {
        // horizontal lines
        QPointF leftBottom = snapToNearestPixel(QPointF(4.5, 14.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF rightBottom = snapToNearestPixel(QPointF(13.5, 14.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF leftMiddle = snapToNearestPixel(QPointF(9.5, 10.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF rightMiddle = snapToNearestPixel(QPointF(13.5, 10.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF leftTop = snapToNearestPixel(QPointF(9.5, 6.5), SnapPixel::ToHalf, SnapPixel::ToHalf);
        QPointF rightTop = snapToNearestPixel(QPointF(13.5, 6.5), SnapPixel::ToHalf, SnapPixel::ToHalf);

        // define relative spaces between lines evenly; to correct for any distortion
        qreal verticalSpacing = std::max(leftMiddle.y() - leftTop.y(), leftBottom.y() - leftMiddle.y());
        rightBottom.setY(leftBottom.y());
        rightMiddle.setX(rightTop.x());
        leftMiddle.setY(leftBottom.y() - verticalSpacing);
        rightMiddle.setY(leftMiddle.y());
        rightTop.setX(rightBottom.x());
        leftTop.setX(leftMiddle.x());
        rightTop.setY(rightMiddle.y() - verticalSpacing);
        leftTop.setY(rightTop.y());

        m_painter->drawLine(leftBottom, rightBottom);
        m_painter->drawLine(leftMiddle, rightMiddle);
        m_painter->drawLine(leftTop, rightTop);

        // arrow
        m_painter->drawLine(snapToNearestPixel(QPointF(4.5, 4.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down),
                            snapToNearestPixel(QPointF(4.5, 12.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down));

        QVector<QPointF> arrowHead{snapToNearestPixel(QPointF(2.5, 10.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down),
                                   snapToNearestPixel(QPointF(4.5, 12.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down),
                                   snapToNearestPixel(QPointF(6.5, 10.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down)};

        if (flatCapArrowHead)
            arrowHead[0] = snapToNearestPixel(QPointF(2, 10), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);
        qreal arrowSlopeWidth = arrowHead[1].x() - arrowHead[0].x();
        arrowHead[2] = QPointF(arrowHead[1].x() + arrowSlopeWidth,
                               arrowHead[0].y()); // set the other side of arrowhead in relative co-ordinates in case pixel-snapping is uneven either side
        m_painter->drawPolyline(arrowHead);

    } else {
        // horizontal lines
        QPointF leftBottom = snapToNearestPixel(QPointF(4.5, 14.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF rightBottom = snapToNearestPixel(QPointF(13.5, 14.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF leftMiddle = snapToNearestPixel(QPointF(9.5, 10.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF rightMiddle = snapToNearestPixel(QPointF(13.5, 10.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF leftTop = snapToNearestPixel(QPointF(9.5, 6.5), SnapPixel::ToWhole, SnapPixel::ToWhole);
        QPointF rightTop = snapToNearestPixel(QPointF(13.5, 6.5), SnapPixel::ToWhole, SnapPixel::ToWhole);

        // define relative spaces between lines evenly; to correct for any distortion
        qreal verticalSpacing = std::max(leftMiddle.y() - leftTop.y(), leftBottom.y() - leftMiddle.y());
        rightBottom.setY(leftBottom.y());
        rightMiddle.setX(rightTop.x());
        leftMiddle.setY(leftBottom.y() - verticalSpacing);
        rightMiddle.setY(leftMiddle.y());
        rightTop.setX(rightBottom.x());
        leftTop.setX(leftMiddle.x());
        rightTop.setY(rightMiddle.y() - verticalSpacing);
        leftTop.setY(rightTop.y());

        m_painter->drawLine(leftBottom, rightBottom);
        m_painter->drawLine(leftMiddle, rightMiddle);
        m_painter->drawLine(leftTop, rightTop);

        // arrow
        m_painter->drawLine(snapToNearestPixel(QPointF(4.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down),
                            snapToNearestPixel(QPointF(4.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down));

        QVector<QPointF> arrowHead{snapToNearestPixel(QPointF(2.5, 10.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down),
                                   snapToNearestPixel(QPointF(4.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down),
                                   snapToNearestPixel(QPointF(6.5, 10.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down)};

        if (flatCapArrowHead)
            arrowHead[0] = snapToNearestPixel(QPointF(2, 10), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
        qreal arrowSlopeWidth = arrowHead[1].x() - arrowHead[0].x();
        arrowHead[2] = QPointF(arrowHead[1].x() + arrowSlopeWidth,
                               arrowHead[0].y()); // set the other side of arrowhead in relative co-ordinates in case pixel-snapping is uneven either side
        m_painter->drawPolyline(arrowHead);
    }
}

void RenderDecorationButtonIcon18By18::renderKeepInFrontIconAsFromBreezeIcons()
{
    QPen pen = m_painter->pen();

    bool flatCapArrowHead = false;
    if (m_totalScalingFactor < 1.3) {
        pen.setCapStyle(Qt::FlatCap); // prevents blurry lobsided arrow at loDPI
        flatCapArrowHead = true;
    }

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.2);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        pen.setWidthF(roundedBoldPenWidth);
    }
    m_painter->setPen(pen);

    if (isOddPenWidth) {
        // horizontal lines
        QPointF leftTop = snapToNearestPixel(QPointF(4.5, 4.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
        QPointF rightTop = snapToNearestPixel(QPointF(13.5, 4.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
        QPointF leftMiddle = snapToNearestPixel(QPointF(4.5, 8.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
        QPointF rightMiddle = snapToNearestPixel(QPointF(8.5, 8.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
        QPointF leftBottom = snapToNearestPixel(QPointF(4.5, 12.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);
        QPointF rightBottom = snapToNearestPixel(QPointF(8.5, 12.5), SnapPixel::ToHalf, SnapPixel::ToHalf, ThresholdRound::Down, ThresholdRound::Down);

        // define relative spaces between lines evenly; to correct for any distortion
        qreal verticalSpacing = std::max(leftMiddle.y() - leftTop.y(), leftBottom.y() - leftMiddle.y());
        rightTop.setY(leftTop.y());
        leftMiddle.setX(leftTop.x());
        leftMiddle.setY(leftTop.y() + verticalSpacing);
        rightMiddle.setY(leftMiddle.y());
        leftBottom.setX(leftTop.x());
        rightBottom.setX(rightMiddle.x());
        leftBottom.setY(leftMiddle.y() + verticalSpacing);
        rightBottom.setY(leftBottom.y());

        m_painter->drawLine(leftTop, rightTop);
        m_painter->drawLine(leftMiddle, rightMiddle);
        m_painter->drawLine(leftBottom, rightBottom);

        // arrow
        m_painter->drawLine(snapToNearestPixel(QPointF(13.5, 6.5), SnapPixel::ToHalf, SnapPixel::ToHalf),
                            snapToNearestPixel(QPointF(13.5, 14.5), SnapPixel::ToHalf, SnapPixel::ToHalf));

        QVector<QPointF> arrowHead{snapToNearestPixel(QPointF(11.5, 8.5), SnapPixel::ToHalf, SnapPixel::ToHalf),
                                   snapToNearestPixel(QPointF(13.5, 6.5), SnapPixel::ToHalf, SnapPixel::ToHalf),
                                   snapToNearestPixel(QPointF(15.5, 8.5), SnapPixel::ToHalf, SnapPixel::ToHalf)};
        if (flatCapArrowHead)
            arrowHead[2] = snapToNearestPixel(QPointF(16, 9), SnapPixel::ToWhole, SnapPixel::ToWhole);
        qreal arrowSlopeWidth = arrowHead[2].x() - arrowHead[1].x();
        arrowHead[0] = QPointF(arrowHead[1].x() - arrowSlopeWidth,
                               arrowHead[2].y()); // set the other side of arrowhead in relative co-ordinates in case pixel-snapping is uneven either side
        m_painter->drawPolyline(arrowHead);
    } else {
        // horizontal lines
        QPointF leftTop = snapToNearestPixel(QPointF(4.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);
        QPointF rightTop = snapToNearestPixel(QPointF(13.5, 4.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);
        QPointF leftMiddle = snapToNearestPixel(QPointF(4.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);
        QPointF rightMiddle = snapToNearestPixel(QPointF(8.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);
        QPointF leftBottom = snapToNearestPixel(QPointF(4.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);
        QPointF rightBottom = snapToNearestPixel(QPointF(8.5, 12.5), SnapPixel::ToWhole, SnapPixel::ToWhole, ThresholdRound::Down, ThresholdRound::Down);

        // define relative spaces between lines evenly; to correct for any distortion
        qreal verticalSpacing = std::max(leftMiddle.y() - leftTop.y(), leftBottom.y() - leftMiddle.y());
        rightTop.setY(leftTop.y());
        leftMiddle.setX(leftTop.x());
        leftMiddle.setY(leftTop.y() + verticalSpacing);
        rightMiddle.setY(leftMiddle.y());
        leftBottom.setX(leftTop.x());
        rightBottom.setX(rightMiddle.x());
        leftBottom.setY(leftMiddle.y() + verticalSpacing);
        rightBottom.setY(leftBottom.y());

        m_painter->drawLine(leftTop, rightTop);
        m_painter->drawLine(leftMiddle, rightMiddle);
        m_painter->drawLine(leftBottom, rightBottom);

        // arrow
        m_painter->drawLine(snapToNearestPixel(QPointF(13.5, 6.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                            snapToNearestPixel(QPointF(13.5, 14.5), SnapPixel::ToWhole, SnapPixel::ToWhole));

        QVector<QPointF> arrowHead{snapToNearestPixel(QPointF(11.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                                   snapToNearestPixel(QPointF(13.5, 6.5), SnapPixel::ToWhole, SnapPixel::ToWhole),
                                   snapToNearestPixel(QPointF(15.5, 8.5), SnapPixel::ToWhole, SnapPixel::ToWhole)};
        if (flatCapArrowHead)
            arrowHead[2] = snapToNearestPixel(QPointF(16, 9), SnapPixel::ToHalf, SnapPixel::ToHalf);
        qreal arrowSlopeWidth = arrowHead[2].x() - arrowHead[1].x();
        arrowHead[0] = QPointF(arrowHead[1].x() - arrowSlopeWidth,
                               arrowHead[2].y()); // set the other side of arrowhead in relative co-ordinates in case pixel-snapping is uneven either side
        m_painter->drawPolyline(arrowHead);
    }
}

void RenderDecorationButtonIcon18By18::renderRounderAndBolderContextHelpIcon()
{
    QPen pen = m_painter->pen();

    if ((!m_fromKstyle) && m_boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.6);
    }

    pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(pen);

    // main body of question mark
    QPainterPath path;
    path.moveTo(7, 5);
    path.arcTo(QRectF(6.5, 3.5, 5.5, 5), 150, -160);
    path.cubicTo(QPointF(12, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    m_painter->drawPath(path);

    // dot of question mark
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(pen.color());
    if ((!m_fromKstyle) && m_boldButtonIcons)
        m_painter->drawEllipse(QRectF(8, 14, 2, 2));
    else
        m_painter->drawEllipse(QRectF(8.25, 14.25, 1.5, 1.5));
}

}
