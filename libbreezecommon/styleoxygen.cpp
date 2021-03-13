/*
 * Copyright 2017  Paul McAuley <kde@paulmcauley.com>
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
