/*
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleoxygen.h"

namespace Breeze
{
    void RenderStyleOxygen18By18::renderCloseIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.75 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderCloseIcon();
    }
    
    void RenderStyleOxygen18By18::renderMaximizeIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.75 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderMaximizeIcon();
    }
    
    void RenderStyleOxygen18By18::renderRestoreIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.75 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderRestoreIcon();
    }
    
    void RenderStyleOxygen18By18::renderMinimizeIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.75 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderMinimizeIcon();
    }
    
    void RenderStyleOxygen18By18::renderKeepBehindIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.4 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderKeepBehindIcon();
    }
    
    void RenderStyleOxygen18By18::renderKeepInFrontIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.4 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderKeepInFrontIcon();
    }
    
    void RenderStyleOxygen18By18::renderContextHelpIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.6 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderContextHelpIcon();
    }
    
    void RenderStyleOxygen18By18::renderShadeIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.6 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderShadeIcon();
    }
    
    void RenderStyleOxygen18By18::renderUnShadeIcon()
    {
        if( (!notInTitlebar) && boldButtonIcons ){
            pen.setWidthF( pen.widthF() *1.6 );
            painter->setPen( pen );
        }
        
        RenderDecorationButtonIcon18By18::renderUnShadeIcon();
    }
    
}
