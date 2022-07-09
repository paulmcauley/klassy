/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleclassik.h"

namespace Breeze
{
void RenderStyleClassik18By18::renderCloseIcon()
{
    pen.setWidthF(pen.widthF() * 1.166666666); // thicken up diagonal slightly to give a balanced look
    painter->setPen(pen);

    if (notInTitlebar) {
        RenderDecorationButtonIcon18By18::renderCloseIcon();

    } else {
        if (boldButtonIcons) {
            pen.setWidthF(pen.widthF() * 1.5); // total factor of 1.75
            painter->setPen(pen);
        }

        // slightly larger X to tie-in with design of square maximize button
        painter->drawLine(QPointF(4.5, 4.5), QPointF(13.5, 13.5));
        painter->drawLine(QPointF(13.5, 4.5), QPointF(4.5, 13.5));
    }
}

void RenderStyleClassik18By18::renderMaximizeIcon()
{
    if (!notInTitlebar) {
        if (boldButtonIcons) {
            // thicker pen in titlebar
            pen.setWidthF(pen.widthF() * 1.666666);
        }
    }

    pen.setJoinStyle(Qt::BevelJoin);
    painter->setPen(pen);
    // large square
    painter->drawRoundedRect(QRectF(QPointF(4.5, 4.5), QPointF(13.5, 13.5)), 0.025, 0.025, Qt::RelativeSize);
    // painter->drawRect( QRectF( QPointF( 4.5, 4.5 ), QPointF( 13.5, 13.5 ) ) );
}

void RenderStyleClassik18By18::renderRestoreIcon()
{
    pen.setJoinStyle(Qt::BevelJoin);
    painter->setPen(pen);

    if (this->notInTitlebar) {
        // disable antialiasing to remove blur at small sizes
        painter->setRenderHints(QPainter::Antialiasing, false);

        // overlapping windows icon
        painter->drawRect(QRectF(QPointF(4, 6), QPointF(11, 13)));
        painter->drawPolyline(QVector<QPointF>{QPointF(6, 6), QPointF(6, 4), QPointF(13, 4), QPointF(13, 11), QPointF(11, 11)});

    } else {
        if (boldButtonIcons) {
            // thicker pen in titlebar
            pen.setWidthF(pen.widthF() * 1.3);
            painter->setPen(pen);
        }

        // overlapping windows icon
        // foregreound square
        painter->drawRect(QRectF(QPointF(4.5, 6.5), QPointF(11.5, 13.5)));

        // background square
        painter->drawPolyline(QVector<QPointF>{QPointF(6.5, 6), QPointF(6.5, 4.5), QPointF(13.5, 4.5), QPointF(13.5, 11.5), QPointF(12, 11.5)});
    }
}

void RenderStyleClassik18By18::renderMinimizeIcon()
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

/*//Experimental 3 squares
    void RenderStyleClassik18By18::renderKeepBehindIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //foreground squares
        painter->drawRect( QRectF( QPointF( 3.5, 3.5 ), QPointF( 8.5, 8.5 ) ) );
        painter->drawRect( QRectF( QPointF( 9.5, 9.5 ), QPointF( 14.5, 14.5 ) ) );

        //filled background square
        painter->setBrush( pen.color() );
        painter->drawPolygon( QPolygonF()
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

    void RenderStyleClassik18By18::renderKeepInFrontIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //background squares
        painter->drawRect( QRectF( QPointF( 3.5, 3.5 ), QPointF( 8.5, 8.5 ) ) );
        painter->drawRect( QRectF( QPointF( 9.5, 9.5 ), QPointF( 14.5, 14.5 ) ) );

        //filled foreground square
        painter->setBrush( pen.color() );
        painter->drawRect( QRectF( QPointF( 5.5, 5.5 ), QPointF( 12.5, 12.5 ) ) );
    }
*/

/*//Experimental 2 squares
    void RenderStyleClassik18By18::renderKeepBehindIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //foreground square
        painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 13.5, 13.5 ) ) );

        //filled background square
        painter->setBrush( pen.color() );
        painter->drawPolygon( QPolygonF()
            << QPointF( 4.5, 4.5 )
            << QPointF( 10.5, 4.5 )
            << QPointF( 10.5, 7.5 )
            << QPointF( 7.5, 7.5 )
            << QPointF( 7.5, 10.5 )
            << QPointF( 4.5, 10.5 )
        );

    }

    void RenderStyleClassik18By18::renderKeepInFrontIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //background square
        painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 13.5, 13.5 ) ) );

        //filled foreground square
        painter->setBrush( pen.color() );
        painter->drawRect( QRectF( QPointF( 4.5, 4.5 ), QPointF( 10.5, 10.5 ) ) );
    }
*/

/* //Experimental filled arrows
    void RenderStyleClassik18By18::renderKeepBehindIcon()
    {
        //horizontal lines
        painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
        painter->drawLine( QPointF( 9.5, 9.5 ), QPointF( 13.5, 9.5 ) );
        painter->drawLine( QPointF( 9.5, 5.5 ), QPointF( 13.5, 5.5 ) );

        //arrow
        painter->drawLine( QPointF( 4.5, 3.5 ), QPointF( 4.5, 11.5 ) );

        painter->setBrush( pen.color() );
        painter->drawConvexPolygon( QPolygonF()
            << QPointF( 2.5, 8.5 )
            << QPointF( 4.5, 11.5 )
            << QPointF( 6.5, 8.5 ) );
    }

    void RenderStyleClassik18By18::renderKeepInFrontIcon()
    {
        //horizontal lines
        painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
        painter->drawLine( QPointF( 4.5, 8.5 ), QPointF( 8.5, 8.5 ) );
        painter->drawLine( QPointF( 4.5, 12.5 ), QPointF( 8.5, 12.5 ) );

        //arrow
        painter->drawLine( QPointF( 13.5, 6.5 ), QPointF( 13.5, 14.5 ) );

        painter->setBrush( pen.color() );
        painter->drawConvexPolygon( QPolygonF()
            << QPointF( 11.5, 9.5 )
            << QPointF( 13.5, 6.5 )
            << QPointF( 15.5, 9.5 ) );
    }
*/

// For consistency with breeze icon set
void RenderStyleClassik18By18::renderKeepBehindIcon()
{
    if ((!notInTitlebar) && boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.1);
        painter->setPen(pen);
    }

    // horizontal lines
    painter->drawLine(QPointF(4.5, 13.5), QPointF(13.5, 13.5));
    painter->drawLine(QPointF(9.5, 9.5), QPointF(13.5, 9.5));
    painter->drawLine(QPointF(9.5, 5.5), QPointF(13.5, 5.5));

    // arrow
    painter->drawLine(QPointF(4.5, 3.5), QPointF(4.5, 11.5));

    painter->drawPolyline(QVector<QPointF>{QPointF(2.5, 9.5), QPointF(4.5, 11.5), QPointF(6.5, 9.5)});
}

void RenderStyleClassik18By18::renderKeepInFrontIcon()
{
    if ((!notInTitlebar) && boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.1);
        painter->setPen(pen);
    }

    // horizontal lines
    painter->drawLine(QPointF(4.5, 4.5), QPointF(13.5, 4.5));
    painter->drawLine(QPointF(4.5, 8.5), QPointF(8.5, 8.5));
    painter->drawLine(QPointF(4.5, 12.5), QPointF(8.5, 12.5));

    // arrow
    painter->drawLine(QPointF(13.5, 6.5), QPointF(13.5, 14.5));

    painter->drawPolyline(QVector<QPointF>{QPointF(11.5, 8.5), QPointF(13.5, 6.5), QPointF(15.5, 8.5)});
}

void RenderStyleClassik18By18::renderContextHelpIcon()
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

void RenderStyleClassik18By18::renderShadeIcon()
{
    if ((!notInTitlebar) && boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.3);
        painter->setPen(pen);
    }
    RenderDecorationButtonIcon18By18::renderShadeIcon();
}

void RenderStyleClassik18By18::renderUnShadeIcon()
{
    if ((!notInTitlebar) && boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.3);
        painter->setPen(pen);
    }
    RenderDecorationButtonIcon18By18::renderUnShadeIcon();
}
}
