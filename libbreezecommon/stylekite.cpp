/*
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylekite.h"

namespace Breeze
{
    void RenderStyleKite18By18::renderCloseIcon()
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
    
    void RenderStyleKite18By18::renderMaximizeIcon()
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
    
    void RenderStyleKite18By18::renderRestoreIcon()
    {
        
        if (this->notInTitlebar) { // slightly smaller diamond
            pen.setJoinStyle( Qt::RoundJoin );
            painter->setPen( pen );
            
            //diamond / floating kite
            painter->drawConvexPolygon( QVector<QPointF>{
                QPointF( 4.5, 9 ),
                QPointF( 9, 4.5 ),
                QPointF( 13.5, 9 ),
                QPointF( 9, 13.5 )});
            
        } else {
            //thicker pen in titlebar
            if(boldButtonIcons){
                pen.setWidthF( pen.widthF() *1.75 );
            } 
            pen.setJoinStyle( Qt::RoundJoin );
            painter->setPen( pen );
           
            //diamond / floating kite
            painter->drawConvexPolygon( QVector<QPointF>{
                QPointF( 4.5, 9 ),
                QPointF( 9, 4.5 ),
                QPointF( 13.5, 9 ),
                QPointF( 9, 13.5 )});
            
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
        //tiny filled square
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setBrush( pen.color() );
        painter->setPen( pen );

        painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 10.5, 10.5 ) ) );
    }
    

// For consistency with breeze icon set
    void RenderStyleKite18By18::renderKeepBehindIcon()
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
    
    void RenderStyleKite18By18::renderKeepInFrontIcon()
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

    
    void RenderStyleKite18By18::renderContextHelpIcon()
    {
        //thicker pen in titlebar
        if((!notInTitlebar) && boldButtonIcons) {
            pen.setWidthF( pen.widthF() *1.6 );
        }
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );
        
        QPainterPath path;
        path.moveTo( 7, 5 );
        path.arcTo( QRectF( 6.5, 3.5, 5.5, 5 ), 150, -160 );
        path.cubicTo( QPointF(12, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
        painter->drawPath( path );
        
        painter->setPen( Qt::NoPen );
        painter->setBrush( pen.color() );
        painter->drawEllipse( QRectF( 8, 14, 2, 2 ) );
    }

    void RenderStyleKite18By18::renderShadeIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.3 );
            painter->setPen( pen );
        }
        RenderDecorationButtonIcon18By18::renderShadeIcon();
    }
    
    void RenderStyleKite18By18::renderUnShadeIcon()
    {        
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.3 );
            painter->setPen( pen );
        }
        RenderDecorationButtonIcon18By18::renderUnShadeIcon();
    }
    
}
