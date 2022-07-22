/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekite.h"

namespace Breeze
{
void RenderStyleKite18By18::renderCloseIcon()
{
    // first determine whether the close button should be enlarged to match an enlarged maximized button
    bool isSmallerMaximize = true;
    int roundedBoldPenWidth = 1;
    if (!notInTitlebar) {
        if (boldButtonIcons)
            isSmallerMaximize = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_maximizeBoldPenWidthFactor);
        else
            isSmallerMaximize = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
    }

    pen.setWidthF(pen.widthF() * 1.166666666); // thicken up diagonal slightly to give a balanced look
    painter->setPen(pen);

    if (notInTitlebar) {
        RenderDecorationButtonIcon18By18::renderCloseIcon();

    } else {
        if (boldButtonIcons) {
            pen.setWidthF(pen.widthF() * 1.5); // total factor of 1.75
            painter->setPen(pen);
        }

        if (isSmallerMaximize) {
            // slightly larger X than Breeze to tie-in with design of square maximize button
            painter->drawLine(QPointF(4.5, 4.5), QPointF(13.5, 13.5));
            painter->drawLine(QPointF(13.5, 4.5), QPointF(4.5, 13.5));
        } else {
            qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);

            // very slightly larger X again to match larger maximize button
            painter->drawLine(QPointF(4.5 - adjustmentOffset, 4.5 - adjustmentOffset), QPointF(13.5 + adjustmentOffset, 13.5 + adjustmentOffset));
            painter->drawLine(QPointF(13.5 + adjustmentOffset, 4.5 - adjustmentOffset), QPointF(4.5 - adjustmentOffset, 13.5 + adjustmentOffset));
        }
    }
}

void RenderStyleKite18By18::renderMaximizeIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_maximizeBoldPenWidthFactor);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth);

        painter->setPen(pen);
    }

    // large square
    QRectF rect(QPointF(4.5, 4.5), QPointF(13.5, 13.5));
    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    painter->drawRoundedRect(rect, 0.025, 0.025, Qt::RelativeSize);
}

void RenderStyleKite18By18::renderRestoreIcon()
{
    if (this->notInTitlebar) { // slightly smaller diamond
        pen.setJoinStyle(Qt::RoundJoin);
        painter->setPen(pen);

        // diamond / floating kite
        painter->drawConvexPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)});

    } else {
        // thicker pen in titlebar
        if (boldButtonIcons) {
            pen.setWidthF(pen.widthF() * 1.75);
        }
        pen.setJoinStyle(Qt::RoundJoin);
        painter->setPen(pen);

        // diamond / floating kite
        painter->drawConvexPolygon(QVector<QPointF>{QPointF(4.5, 9), QPointF(9, 4.5), QPointF(13.5, 9), QPointF(9, 13.5)});

        /* //original large diamond
        painter->drawConvexPolygon( QVector<QPointF>{
            QPointF( 4, 9 ),
            QPointF( 9, 4 ),
            QPointF( 14, 9 ),
            QPointF( 9, 14 )} );
        */
    }
}

void RenderStyleKite18By18::renderMinimizeIcon()
{
    if (boldButtonIcons) {
        // tiny filled square
        pen.setJoinStyle(Qt::BevelJoin);
        painter->setBrush(pen.color());
        painter->setPen(pen);

        painter->drawRect(QRectF(QPointF(7.5, 7.5), QPointF(10.5, 10.5)));

    } else { // in fine mode the dense minimize button appears bolder than the others so reduce its opacity to compensate
        QColor penColor = pen.color();
        QColor brushColor = penColor;
        brushColor.setAlphaF(brushColor.alphaF() * 0.75);
        penColor.setAlphaF(penColor.alphaF() * 0.6);
        pen.setColor(penColor);

        // tiny filled square
        pen.setJoinStyle(Qt::BevelJoin);
        painter->setBrush(brushColor);
        painter->setPen(pen);

        painter->drawRect(QRectF(QPointF(7.5, 7.5), QPointF(10.5, 10.5)));
    }
}

// For consistency with breeze icon set
void RenderStyleKite18By18::renderKeepBehindIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.1);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        pen.setWidthF(roundedBoldPenWidth);
        painter->setPen(pen);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, -adjustmentOffset));
    }

    // horizontal lines
    painter->drawLine(QPointF(4.5, 14.5), QPointF(13.5, 14.5));
    painter->drawLine(QPointF(9.5, 10.5), QPointF(13.5, 10.5));
    painter->drawLine(QPointF(9.5, 6.5), QPointF(13.5, 6.5));

    // arrow
    painter->drawLine(QPointF(4.5, 4.5), QPointF(4.5, 12.5));

    painter->drawPolyline(QVector<QPointF>{QPointF(2.5, 10.5), QPointF(4.5, 12.5), QPointF(6.5, 10.5)});
}

void RenderStyleKite18By18::renderKeepInFrontIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.1);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        pen.setWidthF(roundedBoldPenWidth);
        painter->setPen(pen);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, -adjustmentOffset));
    }

    // horizontal lines
    painter->drawLine(QPointF(4.5, 4.5), QPointF(13.5, 4.5));
    painter->drawLine(QPointF(4.5, 8.5), QPointF(8.5, 8.5));
    painter->drawLine(QPointF(4.5, 12.5), QPointF(8.5, 12.5));

    // arrow
    painter->drawLine(QPointF(13.5, 6.5), QPointF(13.5, 14.5));

    painter->drawPolyline(QVector<QPointF>{QPointF(11.5, 8.5), QPointF(13.5, 6.5), QPointF(15.5, 8.5)});
}

void RenderStyleKite18By18::renderContextHelpIcon()
{
    if ((!notInTitlebar) && boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.6);
    }

    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    // main body of question mark
    QPainterPath path;
    path.moveTo(7, 5);
    path.arcTo(QRectF(6.5, 3.5, 5.5, 5), 150, -160);
    path.cubicTo(QPointF(12, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    painter->drawPath(path);

    // dot of question mark
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen.color());
    if ((!notInTitlebar) && boldButtonIcons)
        painter->drawEllipse(QRectF(8, 14, 2, 2));
    else
        painter->drawEllipse(QRectF(8.25, 14.25, 1.5, 1.5));
}
}
