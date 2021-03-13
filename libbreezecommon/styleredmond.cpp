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

#include "styleredmond.h"

namespace Breeze
{
    void RenderStyleRedmond18By18::renderCloseIcon()
    {
        if(notInTitlebar) {
            
            RenderDecorationButtonIcon18By18::renderCloseIcon();
            
        } else {
            if(boldButtonIcons) {
                //thicker pen in titlebar
                pen.setWidthF( pen.widthF() *1.75 );
                painter->setPen( pen );
            }
            
            // slightly larger X to tie-in with design of square maximize button
            painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 13.5 ) );
            painter->drawLine( QPointF(13.5, 4.5), QPointF(4.5, 13.5) );
        }
    }

    void RenderStyleRedmond18By18::renderMaximizeIcon()
    {
        if(!notInTitlebar) {
            
            if(boldButtonIcons) {
                //thicker pen in titlebar
                pen.setWidthF( pen.widthF() *1.666666 );
            }
            
        }
        
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );
        //large square
        painter->drawRoundedRect( QRectF( QPointF( 4.5, 4.5 ), QPointF( 13.5, 13.5 ) ), 0.025, 0.025, Qt::RelativeSize);
    }

    void RenderStyleRedmond18By18::renderRestoreIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );
        
        if(this->notInTitlebar){
            //disable antialiasing to remove blur at small sizes
            painter->setRenderHints( QPainter::Antialiasing, false );
           
            //overlapping windows icon
            painter->drawRect( QRectF( QPointF( 4, 6 ), QPointF( 11, 13 ) ) );
            painter->drawPolyline( QVector<QPointF>{
                QPointF( 6, 6 ),
                QPointF( 6, 4 ),
                QPointF( 13, 4 ),
                QPointF( 13, 11 ),
                QPointF( 11, 11 )} );
            
        } else {
            if(boldButtonIcons) {
                //thicker pen in titlebar
                pen.setWidthF( pen.widthF() *1.3 );
                painter->setPen( pen );
            }
            
            //overlapping windows icon
            painter->drawRect( QRectF( QPointF( 4.5, 6.5 ), QPointF( 11.5, 13.5 ) ) );
            painter->drawPolyline( QVector<QPointF>{
                QPointF( 6.5, 6.5 ),
                QPointF( 6.5, 4.5 ),
                QPointF( 13.5, 4.5 ),
                QPointF( 13.5, 11.5 ),
                QPointF( 11.5, 11.5 )} );
        }
    }
    
    void RenderStyleRedmond18By18::renderMinimizeIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons) {
                //thicker pen in titlebar
                pen.setWidthF( pen.widthF() *1.75 );
                painter->setPen( pen );
        }
        
        //horizontal line
        painter->drawLine( QPointF( 4.5, 9.5 ), QPointF( 13.5, 9.5 ) );
        
    }

    
/*//Experimental 3 squares
    void RenderStyleRedmond18By18::renderKeepBehindIcon()
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
    
    void RenderStyleRedmond18By18::renderKeepInFrontIcon()
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
    void RenderStyleRedmond18By18::renderKeepBehindIcon()
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
    
    void RenderStyleRedmond18By18::renderKeepInFrontIcon()
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

// For consistency with breeze icon set
    void RenderStyleRedmond18By18::renderKeepBehindIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.1 );
            painter->setPen( pen );
        }
        
        //horizontal lines
        painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
        painter->drawLine( QPointF( 9.5, 9.5 ), QPointF( 13.5, 9.5 ) );
        painter->drawLine( QPointF( 9.5, 5.5 ), QPointF( 13.5, 5.5 ) );
        
        //arrow
        painter->drawLine( QPointF( 4.5, 3.5 ), QPointF( 4.5, 11.5 ) );
        
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 2.5, 9.5 ),
            QPointF( 4.5, 11.5 ),
            QPointF( 6.5, 9.5 )} );
    }
    
    void RenderStyleRedmond18By18::renderKeepInFrontIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.1 );
            painter->setPen( pen );
        }
        
        //horizontal lines
        painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
        painter->drawLine( QPointF( 4.5, 8.5 ), QPointF( 8.5, 8.5 ) );
        painter->drawLine( QPointF( 4.5, 12.5 ), QPointF( 8.5, 12.5 ) );
        
        //arrow
        painter->drawLine( QPointF( 13.5, 6.5 ), QPointF( 13.5, 14.5 ) );
        
        painter->drawPolyline( QVector<QPointF>{
            QPointF( 11.5, 8.5 ),
            QPointF( 13.5, 6.5 ),
            QPointF( 15.5, 8.5 )} );
    }
    
    void RenderStyleRedmond18By18::renderContextHelpIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.6 );
        }
        
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );
        
        QPainterPath path;
        path.moveTo( 7, 5 );
        path.arcTo( QRectF( 6.5, 3.5, 5.5, 5 ), 150, -160 );
        path.cubicTo( QPointF(12.5, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
        painter->drawPath( path );
        
        painter->setBrush( pen.color() );
        painter->drawEllipse( QRectF( 9, 15, 0.5, 0.5 ) );
    }
    
    void RenderStyleRedmond18By18::renderShadeIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.3 );
            painter->setPen( pen );
        }
        RenderDecorationButtonIcon18By18::renderShadeIcon();
    }
    
    void RenderStyleRedmond18By18::renderUnShadeIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.3 );
            painter->setPen( pen );
        }
        RenderDecorationButtonIcon18By18::renderUnShadeIcon();
    }
}
