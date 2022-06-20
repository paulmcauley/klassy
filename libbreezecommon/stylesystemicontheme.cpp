/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylesystemicontheme.h"
#include <QIcon>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsColorizeEffect>

namespace Breeze
{
    
    void RenderStyleSystemIconTheme::paintIconFromSystemTheme(QString iconName)
    {
        
        //QIcon::setThemeName(QIcon::themeName()); //doing this hack allows Adwaita icon theme to be partially loaded
        QIcon icon = QIcon::fromTheme(iconName);
        QRect rect(QPoint(0,0),QSize(m_iconWidth,m_iconWidth));
        
        if( m_internalSettings->colorizeSystemIcons() ){
            QGraphicsScene scene;
            QGraphicsPixmapItem item;
            
            /* the following paragraph is a silly workaround to fix a Qt problem with multiple monitors with different DPIs on Wayland
             * When returning a pixmap from a QIcon Qt will give the pixmap the devicePixelRatio of the monitor with the highest devicePixelRatio
             * Qt does not give it the devicePixelRatio of the current monitor. This causes blurry icons on the lower-dpr screens.
             * Therefore have to make an icon scaled by the difference and set the devicePixelRatio manually
             * Qt6 should offer a better solution as has the option to specify the devicePixelRatio when requesting a QPixmap from a QIcon
             */
            QPixmap iconPixmap = icon.pixmap(QSize(m_iconWidth,m_iconWidth));
            int reducedIconWidth = qRound(m_iconWidth * m_devicePixelRatio / iconPixmap.devicePixelRatioF());
            QPixmap iconPixmap2 = icon.pixmap(reducedIconWidth,reducedIconWidth); 
            iconPixmap2.setDevicePixelRatio(m_devicePixelRatio);
            item.setPixmap(iconPixmap2);
            //item.setPixmap(icon.pixmap(QSize(m_iconWidth,m_iconWidth),m_devicePixelRatio)); //need Qt6 for this more straightforward line to work
            
            /* Tint the icon with the pen colour */
            QGraphicsColorizeEffect effect;
            effect.setColor(pen.color());
            item.setGraphicsEffect(&effect);
            
            scene.addItem(&item);
            scene.render(painter,rect,rect);
        } else 
            icon.paint(painter,QRect(QPoint(0,0),QSize(m_iconWidth,m_iconWidth)));
    }
    
    void RenderStyleSystemIconTheme::renderCloseIcon()
    {
        paintIconFromSystemTheme("window-close-symbolic");
    }
    
    void RenderStyleSystemIconTheme::renderMaximizeIcon()
    {
        paintIconFromSystemTheme("window-maximize-symbolic");
    }
    
    void RenderStyleSystemIconTheme::renderRestoreIcon()
    {
        paintIconFromSystemTheme("window-restore-symbolic");
    }
    
    void RenderStyleSystemIconTheme::renderMinimizeIcon()
    {        
        paintIconFromSystemTheme("window-minimize-symbolic");
    }

    void RenderStyleSystemIconTheme::renderKeepBehindIcon()
    {
        paintIconFromSystemTheme("window-keep-below-symbolic");
    }
    
    void RenderStyleSystemIconTheme::renderKeepInFrontIcon()
    {
        paintIconFromSystemTheme("window-keep-above-symbolic");
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
