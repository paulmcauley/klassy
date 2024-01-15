/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"
#include "stylekite.h"
#include "styleklassy.h"
#include "styleoxygen.h"
#include "styleredmond.h"
#include "styleredmond10.h"
#include "systemicontheme.h"
#include <QGraphicsItem>
#include <algorithm>
#include <cmath>

namespace Breeze
{
using KDecoration2::DecorationButtonType;

std::unique_ptr<RenderDecorationButtonIcon18By18> RenderDecorationButtonIcon18By18::factory(const QSharedPointer<Breeze::InternalSettings> internalSettings,
                                                                                            QPainter *painter,
                                                                                            const bool fromKstyle,
                                                                                            const bool boldButtonIcons,
                                                                                            const qreal devicePixelRatio,
                                                                                            const QPointF &deviceOffsetFromZeroReference)
{
    switch (internalSettings->buttonIconStyle()) {
    case InternalSettings::EnumButtonIconStyle::StyleKlassy:
    default:
        return std::make_unique<RenderStyleKlassy18By18>(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetFromZeroReference);

    case InternalSettings::EnumButtonIconStyle::StyleKite:
        return std::make_unique<RenderStyleKite18By18>(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetFromZeroReference);
    case InternalSettings::EnumButtonIconStyle::StyleOxygen:
        return std::make_unique<RenderStyleOxygen18By18>(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetFromZeroReference);
    case InternalSettings::EnumButtonIconStyle::StyleRedmond:
        return std::make_unique<RenderStyleRedmond18By18>(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetFromZeroReference);
    case InternalSettings::EnumButtonIconStyle::StyleRedmond10:
        return std::make_unique<RenderStyleRedmond1018By18>(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetFromZeroReference);
    }
}

RenderDecorationButtonIcon18By18::RenderDecorationButtonIcon18By18(QPainter *painter,
                                                                   const bool fromKstyle,
                                                                   const bool boldButtonIcons,
                                                                   const qreal devicePixelRatio,
                                                                   const QPointF &deviceOffsetFromZeroReference)
    : m_painter(painter)
    , m_pen(painter->pen())
    , m_fromKstyle(fromKstyle)
    , m_boldButtonIcons(boldButtonIcons)
    , m_devicePixelRatio(devicePixelRatio)
    , m_deviceOffsetFromZeroReference(deviceOffsetFromZeroReference)
{
    m_painter->save();
    initPainter();
}

RenderDecorationButtonIcon18By18::~RenderDecorationButtonIcon18By18()
{
    m_painter->restore();
}

void RenderDecorationButtonIcon18By18::initPainter()
{
    m_pen.setCapStyle(Qt::RoundCap);
    m_pen.setJoinStyle(Qt::MiterJoin);
    m_painter->setPen(m_pen);
    m_painter->setBrush(Qt::NoBrush);

    m_totalScalingFactor = m_painter->deviceTransform().m22();
}

void RenderDecorationButtonIcon18By18::renderIcon(KDecoration2::DecorationButtonType type, bool checked)
{
    switch (type) {
    case DecorationButtonType::Close:
        renderCloseIcon();
        break;

    case DecorationButtonType::Maximize:
        if (checked) {
            renderRestoreIcon();
        } else {
            renderMaximizeIcon();
        }
        break;

    case DecorationButtonType::Minimize:
        renderMinimizeIcon();
        break;

    case DecorationButtonType::OnAllDesktops:
        if (checked) {
            renderPinnedOnAllDesktopsIcon();
        } else {
            renderPinOnAllDesktopsIcon();
        }
        break;

    case DecorationButtonType::Shade:
        if (checked) {
            renderUnShadeIcon();
        } else {
            renderShadeIcon();
        }
        break;

    case DecorationButtonType::KeepBelow:
        renderKeepBehindIcon();
        break;

    case DecorationButtonType::KeepAbove:
        renderKeepInFrontIcon();
        break;

    case DecorationButtonType::ApplicationMenu:
        renderApplicationMenuIcon();
        break;

    case DecorationButtonType::ContextHelp:
        renderContextHelpIcon();
        break;

    default:
        break;
    }
}

/* base methods here are KDE's default Breeze/Oxygen style -- override with other styles */
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
    m_pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(m_pen);

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
    QColor color = m_pen.color();
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
    QColor color = m_pen.color();
    color.setAlphaF(color.alphaF() * 0.8);
    m_painter->setBrush(color);
    m_painter->setPen(Qt::NoPen);
    m_painter->drawConvexPolygon(QVector<QPointF>{QPointF(6.5, 8.5), QPointF(12, 3), QPointF(15, 6), QPointF(9.5, 11.5)});

    m_pen.setColor(color);
    m_painter->setPen(m_pen);
    m_painter->drawLine(QPointF(5.5, 7.5), QPointF(10.5, 12.5));
    m_painter->drawLine(QPointF(8, 10), QPointF(4.5, 13.5));
}

void RenderDecorationButtonIcon18By18::renderShadeIcon()
{
    if (m_totalScalingFactor < 1.3)
        m_pen.setCapStyle(Qt::FlatCap); // prevents lobsided arrow at loDPI

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1.3);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        }
        m_pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(m_pen);
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
    if (m_totalScalingFactor < 1.3)
        m_pen.setCapStyle(Qt::FlatCap); // prevents lobsided arrow at loDPI

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1.3);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        }
        m_pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(m_pen);
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

    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        m_pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(m_pen);
    }

    m_painter->setBrush(m_painter->pen().color());

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
    m_pen = m_painter->pen(); // needed as previous function modifies m_pen

    if (m_fromKstyle) {
        m_pen.setWidthF(m_pen.widthF() * 1.2);
    } else if (m_boldButtonIcons) {
        m_pen.setWidthF(qRound(m_pen.widthF() * 1.5));
    }

    m_painter->setPen(m_pen);

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

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, m_squareMaximizeBoldPenWidthFactor);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        }
        m_pen.setWidthF(roundedBoldPenWidth);
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
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    // if size is still smaller than linear to original design, increase again
    if ((rect.width() * m_totalScalingFactor) < (9 * m_totalScalingFactor - 0.0001)) { // 0.0001 as sometimes there are floating point errors
        qreal adjustmentOffset = convertDevicePixelsTo18By18(1);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    if (!returnSizeOnly) {
        // make excessively thick pen widths translucent to balance with other buttons

        qreal opacity = straightLineOpacity();
        QColor penColor = m_pen.color();
        penColor.setAlphaF(penColor.alphaF() * opacity);
        m_pen.setColor(penColor);

        m_painter->setPen(m_pen);
        m_painter->drawRoundedRect(rect, 0.025, 0.025, Qt::RelativeSize);
    } else
        m_painter->restore();

    return rect.height();
}

void RenderDecorationButtonIcon18By18::renderOverlappingWindowsIcon()
{
    m_pen.setJoinStyle(Qt::BevelJoin);
    m_pen.setCapStyle(Qt::FlatCap);

    int roundedBoldPenWidth = 1;
    bool isOddPenWidth = true;
    if (m_boldButtonIcons) {
        isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, m_overlappingWindowsBoldPenWidthFactor);
    } else {
        isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
    }

    // thicker pen in titlebar
    m_pen.setWidthF(roundedBoldPenWidth);

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
    qreal penWidth18By18 = penWidthTo18By18(m_pen);

    // if distance between squares < pen width (factoring in that the background sqaure does not join the foreground at the foreground's centre-point)
    // || distance between squares < 2
    if (((distanceBetweenSquares / penWidth18By18) < 1.25 - 0.0001) ||
        (m_boldButtonIcons && distanceBetweenSquares < 2 - 0.0001) ||
        (!m_boldButtonIcons && distanceBetweenSquares < 1.5 - 0.0001) //0.0001 is because sometimes there are floating point errors
    ) {
        // generate it again using a larger shiftOffset to push apart the squares
        overlappingWindows.reset();
        overlappingWindowsGroup = nullptr;
        foregroundRect = nullptr;
        backgroundPath = nullptr;
        overlappingWindows = calculateOverlappingWindowsScene(isOddPenWidth,
                                                              shiftOffset + convertDevicePixelsTo18By18(1),
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
    QColor penColor = m_pen.color();
    penColor.setAlphaF(penColor.alphaF() * opacity);
    m_pen.setColor(penColor);
    // set the pen widths in all items -- do this at this point as not to have non-cosmetic pen widths in boundingRect().width() calculations above
    foregroundRect->setPen(m_pen);
    backgroundPath->setPen(m_pen);

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
    background[0].setY(foregroundSquare.top() - convertDevicePixelsTo18By18(0.5));

    background[2].setX(background[1].x() + diameter);
    background[2].setY(background[1].y());

    background[3].setX(background[2].x());
    background[3].setY(background[2].y() + diameter);

    background[4].setX(foregroundSquare.right() + convertDevicePixelsTo18By18(0.5));
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
    if (m_boldButtonIcons) {
        QColor penColor = m_pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.7);
        penColor.setAlphaF(penColor.alphaF() * 0.9);
        m_pen.setColor(penColor);

        m_pen.setJoinStyle(Qt::BevelJoin);
        m_painter->setBrush(brushColor);

        isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        m_pen.setWidthF(roundedBoldPenWidth);

        m_painter->setPen(m_pen);
    } else { // in fine mode the dense minimize button appears bolder than the others so reduce its opacity to compensate
        QColor penColor = m_pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.55);
        penColor.setAlphaF(penColor.alphaF() * 0.75);
        m_pen.setColor(penColor);

        m_pen.setJoinStyle(Qt::BevelJoin);
        m_painter->setBrush(brushColor);

        isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        m_pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(m_pen);
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
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    /*
    // if size is still smaller than linear to original design, increase again
    if ((rect.width() * m_totalScalingFactor) < (3 * m_totalScalingFactor - 0.0001)) { // 0.0001 as sometimes there are floating point errors
        qreal adjustmentOffset = convertDevicePixelsTo18By18(1);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }
    */

    m_painter->drawRect(rect);
}

/*//Experimental 3 squares
    void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
    {
        m_pen.setJoinStyle( Qt::RoundJoin );
        m_painter->setPen( m_pen );

        //foreground squares
        m_painter->drawRect( QRectF( QPointF( 3.5, 3.5 ), QPointF( 8.5, 8.5 ) ) );
        m_painter->drawRect( QRectF( QPointF( 9.5, 9.5 ), QPointF( 14.5, 14.5 ) ) );

        //filled background square
        m_painter->setBrush( m_pen.color() );
        m_painter->drawPolygon( QPolygonF()
            << QPointF( 8.5, 5.5 )
            << QPointF( 12.5, 5.5 )
            << QPointF( 12.5, 9.5 )
            << QPointF( 9.5, 9.5 )
            << QPointF( 9.5, 12.5 )
            << QPointF( 5.5, 12.5 )
            << QPointF( 5.5, 8.5 )
            << QPointF( 8.5, 8.5 )
        );
    }

    void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
    {
        m_pen.setJoinStyle( Qt::RoundJoin );
        m_painter->setPen( m_pen );

        //background squares
        m_painter->drawRect( QRectF( QPointF( 3.5, 3.5 ), QPointF( 8.5, 8.5 ) ) );
        m_painter->drawRect( QRectF( QPointF( 9.5, 9.5 ), QPointF( 14.5, 14.5 ) ) );

        //filled foreground square
        m_painter->setBrush( m_pen.color() );
        m_painter->drawRect( QRectF( QPointF( 5.5, 5.5 ), QPointF( 12.5, 12.5 ) ) );
    }
*/

/*//Experimental 2 squares
    void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
    {
        m_pen.setJoinStyle( Qt::RoundJoin );
        m_painter->setPen( m_pen );

        //foreground square
        m_painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 13.5, 13.5 ) ) );

        //filled background square
        m_painter->setBrush( m_pen.color() );
        m_painter->drawPolygon( QPolygonF()
            << QPointF( 4.5, 4.5 )
            << QPointF( 10.5, 4.5 )
            << QPointF( 10.5, 7.5 )
            << QPointF( 7.5, 7.5 )
            << QPointF( 7.5, 10.5 )
            << QPointF( 4.5, 10.5 )
        );

    }

    void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
    {
        m_pen.setJoinStyle( Qt::RoundJoin );
        m_painter->setPen( m_pen );

        //background square
        m_painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 13.5, 13.5 ) ) );

        //filled foreground square
        m_painter->setBrush( m_pen.color() );
        m_painter->drawRect( QRectF( QPointF( 4.5, 4.5 ), QPointF( 10.5, 10.5 ) ) );
    }
*/

/* //Experimental filled arrows
    void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
    {
        //horizontal lines
        m_painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
        m_painter->drawLine( QPointF( 9.5, 9.5 ), QPointF( 13.5, 9.5 ) );
        m_painter->drawLine( QPointF( 9.5, 5.5 ), QPointF( 13.5, 5.5 ) );

        //arrow
        m_painter->drawLine( QPointF( 4.5, 3.5 ), QPointF( 4.5, 11.5 ) );

        m_painter->setBrush( m_pen.color() );
        m_painter->drawConvexPolygon( QPolygonF()
            << QPointF( 2.5, 8.5 )
            << QPointF( 4.5, 11.5 )
            << QPointF( 6.5, 8.5 ) );
    }

    void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
    {
        //horizontal lines
        m_painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
        m_painter->drawLine( QPointF( 4.5, 8.5 ), QPointF( 8.5, 8.5 ) );
        m_painter->drawLine( QPointF( 4.5, 12.5 ), QPointF( 8.5, 12.5 ) );

        //arrow
        m_painter->drawLine( QPointF( 13.5, 6.5 ), QPointF( 13.5, 14.5 ) );

        m_painter->setBrush( m_pen.color() );
        m_painter->drawConvexPolygon( QPolygonF()
            << QPointF( 11.5, 9.5 )
            << QPointF( 13.5, 6.5 )
            << QPointF( 15.5, 9.5 ) );
    }
*/

// For consistency with breeze icon set
void RenderDecorationButtonIcon18By18::renderKeepBehindIconAsFromBreezeIcons()
{
    bool flatCapArrowHead = false;
    if (m_totalScalingFactor < 1.3) {
        m_pen.setCapStyle(Qt::FlatCap); // prevents blurry lobsided arrow at loDPI
        flatCapArrowHead = true;
    }

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1.2);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        m_pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(m_pen);
    }

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
    bool flatCapArrowHead = false;
    if (m_totalScalingFactor < 1.3) {
        m_pen.setCapStyle(Qt::FlatCap); // prevents blurry lobsided arrow at loDPI
        flatCapArrowHead = true;
    }

    bool isOddPenWidth = true;
    if (!m_fromKstyle) {
        int roundedBoldPenWidth = 1;
        if (m_boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1.2);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(m_pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        m_pen.setWidthF(roundedBoldPenWidth);
        m_painter->setPen(m_pen);
    }

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
    if ((!m_fromKstyle) && m_boldButtonIcons) {
        // thicker pen in titlebar
        m_pen.setWidthF(m_pen.widthF() * 1.6);
    }

    m_pen.setJoinStyle(Qt::RoundJoin);
    m_painter->setPen(m_pen);

    // main body of question mark
    QPainterPath path;
    path.moveTo(7, 5);
    path.arcTo(QRectF(6.5, 3.5, 5.5, 5), 150, -160);
    path.cubicTo(QPointF(12, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    m_painter->drawPath(path);

    // dot of question mark
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(m_pen.color());
    if ((!m_fromKstyle) && m_boldButtonIcons)
        m_painter->drawEllipse(QRectF(8, 14, 2, 2));
    else
        m_painter->drawEllipse(QRectF(8.25, 14.25, 1.5, 1.5));
}

bool RenderDecorationButtonIcon18By18::roundedPenWidthIsOdd(const qreal &m_penWidth, int &outputRoundedPenWidth, const qreal boldingFactor)
{
    outputRoundedPenWidth = qRound(m_penWidth * boldingFactor);
    return (outputRoundedPenWidth % 2 != 0);
}

qreal RenderDecorationButtonIcon18By18::convertDevicePixelsTo18By18(const qreal devicePixels)
{
    return (devicePixels / m_totalScalingFactor);
}

void RenderDecorationButtonIcon18By18::translatePainterForAliasedPainting(const bool penWidthOdd)
{
    // see https://doc.qt.io/qt-6/coordsys.html for aliased painting co-ordinates
    if (penWidthOdd) {
        m_painter->translate(QPointF(-0.5, -0.5));
    }
}

qreal RenderDecorationButtonIcon18By18::roundCoordToHalf(qreal coord, const ThresholdRound roundAtZero)
{
    qreal coordIntegralPart, coordFractionalPart;
    static constexpr qreal zeroLimit = 0.0001;

    coordFractionalPart = modf(coord, &coordIntegralPart);

    if (coordFractionalPart > (1 - zeroLimit)) { // at 1 -- same as zero
        coordFractionalPart = 0;

        if (roundAtZero == ThresholdRound::Down) {
            coord = coord - (0.5 + coordFractionalPart);
        } else {
            coord = coord + (0.5 - coordFractionalPart);
        }
    } else if (coordFractionalPart > 0.5) {
        coord = coord - (coordFractionalPart - 0.5);
    } else if (coordFractionalPart < 0.5) {
        if (coordFractionalPart < zeroLimit) { // 0
            coordFractionalPart = 0;

            if (roundAtZero == ThresholdRound::Down) {
                coord = coord - (0.5 + coordFractionalPart);
            } else {
                coord = coord + (0.5 - coordFractionalPart);
            }
        } else {
            coord = coord + (0.5 - coordFractionalPart);
        }
    }

    return coord;
}

qreal RenderDecorationButtonIcon18By18::roundCoordToWhole(qreal coord, const ThresholdRound roundAtHalf)
{
    qreal coordIntegralPart, coordFractionalPart;
    static constexpr qreal halfLimit = 0.0001;

    coordFractionalPart = modf(coord, &coordIntegralPart);

    if (coordFractionalPart > (0.5 + halfLimit) || coordFractionalPart < (0.5 - halfLimit)) {
        coord = round(coord);
    } else {
        if (roundAtHalf == ThresholdRound::Down) {
            coord = floor(coord);
        } else {
            coord = ceil(coord);
        }
    }

    return coord;
}

QPointF RenderDecorationButtonIcon18By18::snapToNearestPixel(QPointF point18By18,
                                                             const SnapPixel snapX,
                                                             const SnapPixel snapY,
                                                             const ThresholdRound roundAtThresholdX,
                                                             const ThresholdRound roundAtThresholdY)
{
    point18By18 *= m_totalScalingFactor;

    // the top-left of the titlebar is used as the reference-point at which the pixel is most likely to be whole
    //(This, however, is not the case with fractional scaling, but cannot get an offset from the top-left of the device screen from the API)
    point18By18 += m_deviceOffsetFromZeroReference;

    if (snapX == SnapPixel::ToHalf) {
        point18By18.setX(roundCoordToHalf(point18By18.x(), roundAtThresholdX));
    } else {
        point18By18.setX(roundCoordToWhole(point18By18.x(), roundAtThresholdX));
    }

    if (snapY == SnapPixel::ToHalf) {
        point18By18.setY(roundCoordToHalf(point18By18.y(), roundAtThresholdY));
    } else {
        point18By18.setY(roundCoordToWhole(point18By18.y(), roundAtThresholdY));
    }

    point18By18 -= m_deviceOffsetFromZeroReference;
    return (point18By18 / m_totalScalingFactor);
}

qreal RenderDecorationButtonIcon18By18::penWidthTo18By18(const QPen &pen)
{
    if (pen.isCosmetic()) {
        return convertDevicePixelsTo18By18(pen.widthF());
    } else {
        return pen.widthF();
    }
}

qreal RenderDecorationButtonIcon18By18::straightLineOpacity()
{
    if (m_devicePixelRatio < 1.2)
        return 0.9;
    else
        return 1;
}
}
