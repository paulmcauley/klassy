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
#include "stylesystemicontheme.h"

namespace Breeze
{

std::unique_ptr<RenderDecorationButtonIcon18By18> RenderDecorationButtonIcon18By18::factory(const QSharedPointer<InternalSettings> internalSettings,
                                                                                            QPainter *painter,
                                                                                            const bool notInTitlebar,
                                                                                            const bool boldButtonIcons,
                                                                                            qreal iconWidth,
                                                                                            qreal devicePixelRatio,
                                                                                            qreal iconScaleFactor)
{
    switch (internalSettings->buttonIconStyle()) {
    case InternalSettings::StyleKlassy:
    default:
        return std::unique_ptr<RenderDecorationButtonIcon18By18>(
            new RenderStyleKlassy18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor));

    case InternalSettings::StyleKite:
        return std::unique_ptr<RenderDecorationButtonIcon18By18>(
            new RenderStyleKite18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor));
    case InternalSettings::StyleOxygen:
        return std::unique_ptr<RenderDecorationButtonIcon18By18>(
            new RenderStyleOxygen18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor));
    case InternalSettings::StyleRedmond:
        return std::unique_ptr<RenderDecorationButtonIcon18By18>(
            new RenderStyleRedmond18By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor));
    case InternalSettings::StyleRedmond10:
        return std::unique_ptr<RenderDecorationButtonIcon18By18>(
            new RenderStyleRedmond1018By18(painter, notInTitlebar, boldButtonIcons, devicePixelRatio, iconScaleFactor));
    case InternalSettings::StyleSystemIconTheme:
        return std::unique_ptr<RenderDecorationButtonIcon18By18>(new RenderStyleSystemIconTheme(painter,
                                                                                                notInTitlebar,
                                                                                                boldButtonIcons,
                                                                                                iconWidth,
                                                                                                internalSettings,
                                                                                                devicePixelRatio,
                                                                                                iconScaleFactor));
    }
}

RenderDecorationButtonIcon18By18::RenderDecorationButtonIcon18By18(QPainter *painter,
                                                                   const bool notInTitlebar,
                                                                   const bool boldButtonIcons,
                                                                   const qreal devicePixelRatio,
                                                                   const qreal iconScaleFactor)
    : painter(painter)
    , pen(painter->pen())
    , notInTitlebar(notInTitlebar)
    , boldButtonIcons(boldButtonIcons)
    , m_devicePixelRatio(devicePixelRatio)
    , m_iconScaleFactor(iconScaleFactor)
{
    painter->save();
    initPainter();
}

RenderDecorationButtonIcon18By18::~RenderDecorationButtonIcon18By18()
{
    painter->restore();
}

void RenderDecorationButtonIcon18By18::initPainter()
{
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
}

/* base methods here are KDE's default Breeze/Oxygen style -- override with other styles */
void RenderDecorationButtonIcon18By18::renderCloseIcon()
{
    painter->drawLine(QPointF(5, 5), QPointF(13, 13));
    painter->drawLine(QPointF(13, 5), QPointF(5, 13));
}

void RenderDecorationButtonIcon18By18::renderMaximizeIcon()
{
    // up arrow
    painter->drawPolyline(QVector<QPointF>{QPointF(4, 11), QPointF(9, 6), QPointF(14, 11)});
}

void RenderDecorationButtonIcon18By18::renderRestoreIcon()
{
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    if (this->notInTitlebar) { // slightly smaller diamond

        // diamond / floating kite
        painter->drawConvexPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)});

    } else {
        // diamond / floating kite
        painter->drawConvexPolygon(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9), QPointF(9, 14)});
    }
}

void RenderDecorationButtonIcon18By18::renderMinimizeIcon()
{
    // down arrow
    painter->drawPolyline(QVector<QPointF>{QPointF(4, 7), QPointF(9, 12), QPointF(14, 7)});
}

void RenderDecorationButtonIcon18By18::renderPinnedOnAllDesktopsIcon()
{
    painter->setBrush(pen.color());
    painter->setPen(Qt::NoPen);

    QPainterPath outerRing;
    outerRing.addEllipse(QRectF(3, 3, 12, 12));

    QPainterPath innerDot;
    innerDot.addEllipse(QRectF(8, 8, 2, 2));

    outerRing = outerRing.subtracted(innerDot);

    painter->drawPath(outerRing);
}

void RenderDecorationButtonIcon18By18::renderPinOnAllDesktopsIcon()
{
    painter->setBrush(pen.color());
    painter->setPen(Qt::NoPen);
    painter->drawConvexPolygon(QVector<QPointF>{QPointF(6.5, 8.5), QPointF(12, 3), QPointF(15, 6), QPointF(9.5, 11.5)});

    painter->setPen(pen);
    painter->drawLine(QPointF(5.5, 7.5), QPointF(10.5, 12.5));
    painter->drawLine(QPointF(12, 6), QPointF(4.5, 13.5));
}

void RenderDecorationButtonIcon18By18::renderShadeIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.2);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth);
        painter->setPen(pen);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, adjustmentOffset));
    }

    painter->drawLine(QPointF(4, 4.5), QPointF(14, 4.5));
    painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
}

void RenderDecorationButtonIcon18By18::renderUnShadeIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.2);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth);
        painter->setPen(pen);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, adjustmentOffset));
    }

    painter->drawLine(QPointF(4, 4.5), QPointF(14, 4.5));
    painter->drawPolyline(QVector<QPointF>{QPointF(4, 8), QPointF(9, 13), QPointF(14, 8)});
}

void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
{
    // two down arrows
    painter->drawPolyline(QVector<QPointF>{QPointF(4, 5), QPointF(9, 10), QPointF(14, 5)});

    painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 14), QPointF(14, 9)});
}

void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
{
    // two up arrows
    painter->drawPolyline(QVector<QPointF>{QPointF(4, 9), QPointF(9, 4), QPointF(14, 9)});

    painter->drawPolyline(QVector<QPointF>{QPointF(4, 13), QPointF(9, 8), QPointF(14, 13)});
}

void RenderDecorationButtonIcon18By18::renderApplicationMenuIcon()
{
    bool isOddPenWidth = true;

    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, -adjustmentOffset));
    }

    painter->drawRect(QRectF(3.5, 4.5, 11, 1));
    painter->drawRect(QRectF(3.5, 8.5, 11, 1));
    painter->drawRect(QRectF(3.5, 12.5, 11, 1));
}

void RenderDecorationButtonIcon18By18::renderContextHelpIcon()
{
    QPainterPath path;
    path.moveTo(5, 6);
    path.arcTo(QRectF(5, 3.5, 8, 5), 180, -180);
    path.cubicTo(QPointF(12.5, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    painter->drawPath(path);

    painter->drawRect(QRectF(9, 15, 0.5, 0.5));
}
bool RenderDecorationButtonIcon18By18::roundedPenWidthIsOdd(const qreal &penWidth, int &outputRoundedPenWidth, const qreal boldingFactor)
{
    outputRoundedPenWidth = qRound(penWidth * boldingFactor);
    return (outputRoundedPenWidth % 2 != 0);
}

qreal RenderDecorationButtonIcon18By18::convertDevicePixelsTo18By18(const qreal devicePixels)
{
    // the totalScalingFactor ensures that the value here converts from the scaled value on screen to the value rendered here in 18px.
    // The dpr used gets the X11 value directly from the KDE scaling settings page, and is not the same as that from the paint device where on X11 it is 1
    qreal totalScalingFactor = m_iconScaleFactor;
    totalScalingFactor *= painter->device()->devicePixelRatioF(); // dpr from the paint device is always 1 on X11
    return (devicePixels / totalScalingFactor);
}
}
