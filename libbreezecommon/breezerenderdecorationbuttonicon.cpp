/*
 * Copyright 2020  Paul McAuley <kde@paulmcauley.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "breezerenderdecorationbuttonicon.h"
#include "breezestyleclassic.h"
#include "breezestyleoxygen.h"
#include "breezestyleredmond.h"

namespace Breeze
{
    
    std::unique_ptr<RenderDecorationButtonIcon18By18> RenderDecorationButtonIcon18By18::factory( QPainter* painter, const QPen& pen, int buttonIconStyle,  const bool notInTitlebar)
    {
        switch( EnumButtonIconStyle(buttonIconStyle) )
        {
            case StyleClassic:
                return std::unique_ptr<RenderDecorationButtonIcon18By18>( new RenderStyleClassic18By18(painter, pen, notInTitlebar) );
            case StyleOxygen:
                default:
                return std::unique_ptr<RenderDecorationButtonIcon18By18>( new RenderStyleOxygen18By18(painter, pen, notInTitlebar) );
            case StyleRedmond:
                return std::unique_ptr<RenderDecorationButtonIcon18By18>( new RenderStyleRedmond18By18(painter, pen, notInTitlebar) );
        }
    }


    
    RenderDecorationButtonIcon18By18::RenderDecorationButtonIcon18By18( QPainter* painter, const QPen& pen, const bool notInTitlebar )
    {
        this->painter = painter;
        this->pen = pen;
        this->notInTitlebar = notInTitlebar;
        
        initPainter();
    }
    
    
    
    void RenderDecorationButtonIcon18By18::initPainter()
    {
        pen.setCapStyle( Qt::RoundCap );
        pen.setJoinStyle( Qt::MiterJoin );
        painter->setPen( pen );
        painter->setBrush( Qt::NoBrush );
    }
    
    
    /* base methods are based on Oxygen style -- override with other styles */
    void RenderDecorationButtonIcon18By18::renderCloseIcon()
    {
        painter->drawLine( 5, 5, 13, 13 );
        painter->drawLine( 13, 5, 5, 13 );
    }
    
    void RenderDecorationButtonIcon18By18::renderMaximizeIcon()
    {
        //up arrow
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 11 ),
            QPointF( 9, 6 ),
            QPointF( 14, 11 )} );
    }
    
    void RenderDecorationButtonIcon18By18::renderRestoreIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );
        
        if (this->notInTitlebar) { // slightly smaller diamond
                
            //diamond / floating kite
            painter->drawConvexPolygon( QVector<QPointF>{
                QPointF( 4.5, 9 ),
                QPointF( 9, 4.5 ),
                QPointF( 13.5, 9 ),
                QPointF( 9, 13.5 )});
            
        } else {
            
            //diamond / floating kite
            painter->drawConvexPolygon( QVector<QPointF>{
                QPointF( 4, 9 ),
                QPointF( 9, 4 ),
                QPointF( 14, 9 ),
                QPointF( 9, 14 )} );           
        }
    }
    
    void RenderDecorationButtonIcon18By18::renderMinimizeIcon()
    {
        //down arrow
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 7 ),
            QPointF( 9, 12 ),
            QPointF( 14, 7 )} );
    }
    
    void RenderDecorationButtonIcon18By18::renderPinnedOnAllDesktopsIcon()
    {
        // thick hollow ring
        pen.setWidthF(5);
        painter->setPen( pen );
        painter->drawEllipse( QPointF( 9, 9 ), qreal(3.5), qreal(3.5) );
    }
    
    void RenderDecorationButtonIcon18By18::renderPinOnAllDesktopsIcon()
    {
        painter->setBrush( pen.color() );
        painter->setPen( Qt::NoPen );
        painter->drawConvexPolygon( QVector<QPointF>{
            QPointF( 6.5, 8.5 ),
            QPointF( 12, 3 ),
            QPointF( 15, 6 ),
            QPointF( 9.5, 11.5 )} );

        painter->setPen( pen );
        painter->drawLine( QPointF( 5.5, 7.5 ), QPointF( 10.5, 12.5 ) );
        painter->drawLine( QPointF( 12, 6 ), QPointF( 4.5, 13.5 ) );
    }
    
    void RenderDecorationButtonIcon18By18::renderShadeIcon()
    {
        painter->drawLine( QPointF( 4, 4.5 ), QPointF( 14, 4.5 ) );
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 13 ),
            QPointF( 9, 8 ),
            QPointF( 14, 13 )} );
    }
    
    void RenderDecorationButtonIcon18By18::renderUnShadeIcon()
    {
        painter->drawLine( QPointF( 4, 4.5 ), QPointF( 14, 4.5 ) );
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 8 ),
            QPointF( 9, 13 ),
            QPointF( 14, 8 )} );
    }
    
    void RenderDecorationButtonIcon18By18::renderKeepBehindIcon()
    {
        //two down arrows
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 5 ),
            QPointF( 9, 10 ),
            QPointF( 14, 5 )} );

        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 9 ),
            QPointF( 9, 14 ),
            QPointF( 14, 9 )} );
    }
    
    void RenderDecorationButtonIcon18By18::renderKeepInFrontIcon()
    {
        //two up arrows
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 9 ),
            QPointF( 9, 4 ),
            QPointF( 14, 9 )} );

        painter->drawPolyline( QVector<QPointF>{
            QPointF( 4, 13 ),
            QPointF( 9, 8 ),
            QPointF( 14, 13 )} );
    }

    void RenderDecorationButtonIcon18By18::renderApplicationMenuIcon()
    {
        painter->drawRect( QRectF( 3.5, 4.5, 11, 1 ) );
        painter->drawRect( QRectF( 3.5, 8.5, 11, 1 ) );
        painter->drawRect( QRectF( 3.5, 12.5, 11, 1 ) );
    }

    void RenderDecorationButtonIcon18By18::renderContextHelpIcon()
    {
        QPainterPath path;
        path.moveTo( 5, 6 );
        path.arcTo( QRectF( 5, 3.5, 8, 5 ), 180, -180 );
        path.cubicTo( QPointF(12.5, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
        painter->drawPath( path );

        painter->drawRect( QRectF( 9, 15, 0.5, 0.5 ) );
    }
    
}
