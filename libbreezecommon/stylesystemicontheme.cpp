/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylesystemicontheme.h"
#include <QIcon>

namespace Breeze
{
    
    void RenderStyleSystemIconTheme::paintQIcon(QIcon& icon)
    {
        icon.paint(painter,QRect(QPoint(0,0),QSize(m_iconWidth,m_iconWidth)));
    }
    
    void RenderStyleSystemIconTheme::renderCloseIcon()
    {
        
        //if(!notInTitlebar) {
        QIcon icon = QIcon::fromTheme("window-close-symbolic");
        paintQIcon(icon);
    }
    
    void RenderStyleSystemIconTheme::renderMaximizeIcon()
    {
        QIcon icon = QIcon::fromTheme("window-maximize-symbolic");
        paintQIcon(icon);
    }
    
    void RenderStyleSystemIconTheme::renderRestoreIcon()
    {
        QIcon icon = QIcon::fromTheme("window-restore-symbolic");
        paintQIcon(icon);
    }
    
    void RenderStyleSystemIconTheme::renderMinimizeIcon()
    {        
        QIcon icon = QIcon::fromTheme("window-minimize-symbolic");
        paintQIcon(icon);
    }

    void RenderStyleSystemIconTheme::renderKeepBehindIcon()
    {
        QIcon icon = QIcon::fromTheme("window-keep-below-symbolic");
        paintQIcon(icon);
    }
    
    void RenderStyleSystemIconTheme::renderKeepInFrontIcon()
    {
        QIcon icon = QIcon::fromTheme("window-keep-above-symbolic");
        paintQIcon(icon);
    }

    void RenderStyleSystemIconTheme::renderContextHelpIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.6 );
        }
        
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );
        
        //main body of question mark
        QPainterPath path;
        path.moveTo( 7, 5 );
        path.arcTo( QRectF( 6.5, 3.5, 5.5, 5 ), 150, -160 );
        path.cubicTo( QPointF(12, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
        painter->drawPath( path );
        
        
        //dot of question mark
        painter->setPen( Qt::NoPen );
        painter->setBrush( pen.color() );
        if((!notInTitlebar) && boldButtonIcons) painter->drawEllipse( QRectF( 8, 14, 2, 2 ) );
        else painter->drawEllipse( QRectF( 8.25, 14.25, 1.5, 1.5 ) );
        
    }
    
    void RenderStyleSystemIconTheme::renderShadeIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.3 );
            painter->setPen( pen );
        }
        RenderDecorationButtonIcon18By18::renderShadeIcon();
    }
    
    void RenderStyleSystemIconTheme::renderUnShadeIcon()
    {
        if((!notInTitlebar) && boldButtonIcons) {
            //thicker pen in titlebar
            pen.setWidthF( pen.widthF() *1.3 );
            painter->setPen( pen );
        }
        RenderDecorationButtonIcon18By18::renderUnShadeIcon();
    }

}
