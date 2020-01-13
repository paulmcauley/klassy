/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "breezetileset.h"

#include <QPainter>

namespace Breeze
{

    //___________________________________________________________
    inline bool bits(TileSet::Tiles flags, TileSet::Tiles testFlags)
    { return (flags & testFlags) == testFlags; }

    //______________________________________________________________________________________
    inline qreal devicePixelRatio( const QPixmap& pixmap )
    {
        return pixmap.devicePixelRatio();
    }

    //______________________________________________________________________________________
    inline void setDevicePixelRatio( QPixmap& pixmap, qreal value )
    {
        return pixmap.setDevicePixelRatio( value );
    }

    //______________________________________________________________
    void TileSet::initPixmap( PixmapList& pixmaps, const QPixmap &source, int width, int height, const QRect &rect)
    {
        QSize size( width, height );
        if( !( size.isValid() && rect.isValid() ) )
        {
            pixmaps.append( QPixmap() );

        } else if( size != rect.size() ) {

            const qreal dpiRatio( devicePixelRatio( source ) );
            const QRect scaledRect( rect.topLeft()*dpiRatio, rect.size()*dpiRatio );
            const QSize scaledSize( size*dpiRatio );
            const QPixmap tile( source.copy(scaledRect) );
            QPixmap pixmap( scaledSize );

            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            painter.drawTiledPixmap(0, 0, scaledSize.width(), scaledSize.height(), tile);
            setDevicePixelRatio( pixmap, dpiRatio );
            pixmaps.append( pixmap );

        } else {

            const qreal dpiRatio( devicePixelRatio( source ) );
            const QRect scaledRect( rect.topLeft()*dpiRatio, rect.size()*dpiRatio );
            QPixmap pixmap( source.copy( scaledRect ) );
            setDevicePixelRatio( pixmap, dpiRatio );
            pixmaps.append( pixmap );

        }

    }

    //______________________________________________________________
    TileSet::TileSet():
        _w1(0),
        _h1(0),
        _w3(0),
        _h3(0)
    { _pixmaps.reserve(9); }

    //______________________________________________________________
    TileSet::TileSet(const QPixmap &source, int w1, int h1, int w2, int h2 ):
        _w1(w1),
        _h1(h1),
        _w3(0),
        _h3(0)
    {
        _pixmaps.reserve(9);
        if( source.isNull() ) return;

        _w3 = source.width()/devicePixelRatio( source ) - (w1 + w2);
        _h3 = source.height()/devicePixelRatio( source ) - (h1 + h2);
        int w = w2;
        int h = h2;

        // initialise pixmap array
        initPixmap( _pixmaps, source, _w1, _h1, QRect(0, 0, _w1, _h1) );
        initPixmap( _pixmaps, source, w, _h1, QRect(_w1, 0, w2, _h1) );
        initPixmap( _pixmaps, source, _w3, _h1, QRect(_w1+w2, 0, _w3, _h1) );
        initPixmap( _pixmaps, source, _w1, h, QRect(0, _h1, _w1, h2) );
        initPixmap( _pixmaps, source, w, h, QRect(_w1, _h1, w2, h2) );
        initPixmap( _pixmaps, source, _w3, h, QRect(_w1+w2, _h1, _w3, h2) );
        initPixmap( _pixmaps, source, _w1, _h3, QRect(0, _h1+h2, _w1, _h3) );
        initPixmap( _pixmaps, source, w, _h3, QRect(_w1, _h1+h2, w2, _h3) );
        initPixmap( _pixmaps, source, _w3, _h3, QRect(_w1+w2, _h1+h2, _w3, _h3) );
    }

    //___________________________________________________________
    void TileSet::render(const QRect &constRect, QPainter *painter, Tiles tiles) const
    {

        const bool oldHint( painter->testRenderHint( QPainter::SmoothPixmapTransform ) );
        painter->setRenderHint( QPainter::SmoothPixmapTransform, true );

        // check initialization
        if( _pixmaps.size() < 9 ) return;

        // copy source rect
        QRect rect( constRect );

        // get rect dimensions
        int x0, y0, w, h;
        rect.getRect(&x0, &y0, &w, &h);

        // calculate pixmaps widths
        int wLeft(0);
        int wRight(0);
        if( _w1+_w3 > 0 )
        {
            qreal wRatio( qreal( _w1 )/qreal( _w1 + _w3 ) );
            wLeft = (tiles&Right) ? qMin( _w1, int(w*wRatio) ):_w1;
            wRight = (tiles&Left) ? qMin( _w3, int(w*(1.0-wRatio)) ):_w3;
        }

        // calculate pixmap heights
        int hTop(0);
        int hBottom(0);
        if( _h1+_h3 > 0 )
        {
            qreal hRatio( qreal( _h1 )/qreal( _h1 + _h3 ) );
            hTop = (tiles&Bottom) ? qMin( _h1, int(h*hRatio) ):_h1;
            hBottom = (tiles&Top) ? qMin( _h3, int(h*(1.0-hRatio)) ):_h3;
        }

        // calculate corner locations
        w -= wLeft + wRight;
        h -= hTop + hBottom;
        const int x1 = x0 + wLeft;
        const int x2 = x1 + w;
        const int y1 = y0 + hTop;
        const int y2 = y1 + h;

        const int w2 = _pixmaps.at(7).width()/devicePixelRatio( _pixmaps.at(7) );
        const int h2 = _pixmaps.at(5).height()/devicePixelRatio( _pixmaps.at(5) );

        // corner
        if( bits( tiles, Top|Left) )  painter->drawPixmap(x0, y0, _pixmaps.at(0), 0, 0, wLeft*devicePixelRatio( _pixmaps.at(0) ), hTop*devicePixelRatio( _pixmaps.at(0) ));
        if( bits( tiles, Top|Right) ) painter->drawPixmap(x2, y0, _pixmaps.at(2), (_w3-wRight)*devicePixelRatio( _pixmaps.at(2) ), 0, wRight*devicePixelRatio( _pixmaps.at(2) ), hTop*devicePixelRatio( _pixmaps.at(2) ) );
        if( bits( tiles, Bottom|Left) )  painter->drawPixmap(x0, y2, _pixmaps.at(6), 0, (_h3-hBottom)*devicePixelRatio( _pixmaps.at(6) ), wLeft*devicePixelRatio( _pixmaps.at(6) ),  hBottom*devicePixelRatio( _pixmaps.at(6) ));
        if( bits( tiles, Bottom|Right) ) painter->drawPixmap(x2, y2, _pixmaps.at(8), (_w3-wRight)*devicePixelRatio( _pixmaps.at(8) ), (_h3-hBottom)*devicePixelRatio( _pixmaps.at(8) ), wRight*devicePixelRatio( _pixmaps.at(8) ), hBottom*devicePixelRatio( _pixmaps.at(8) ) );

        // top and bottom
        if( w > 0 )
        {
            if( tiles&Top ) painter->drawPixmap(x1, y0, w, hTop, _pixmaps.at(1), 0, 0, w2*devicePixelRatio( _pixmaps.at(1) ), hTop*devicePixelRatio( _pixmaps.at(1) ) );
            if( tiles&Bottom ) painter->drawPixmap(x1, y2, w, hBottom, _pixmaps.at(7), 0, (_h3-hBottom)*devicePixelRatio( _pixmaps.at(7) ), w2*devicePixelRatio( _pixmaps.at(7) ), hBottom*devicePixelRatio( _pixmaps.at(7) ) );
        }

        // left and right
        if( h > 0 )
        {
            if( tiles&Left ) painter->drawPixmap(x0, y1, wLeft, h, _pixmaps.at(3), 0, 0, wLeft*devicePixelRatio( _pixmaps.at(3) ), h2*devicePixelRatio( _pixmaps.at(3) ) );
            if( tiles&Right ) painter->drawPixmap(x2, y1, wRight, h, _pixmaps.at(5), (_w3-wRight)*devicePixelRatio( _pixmaps.at(5) ), 0, wRight*devicePixelRatio( _pixmaps.at(5) ), h2*devicePixelRatio( _pixmaps.at(5) ) );
        }

        // center
        if( (tiles&Center) && h > 0 && w > 0 ) painter->drawPixmap(x1, y1, w, h, _pixmaps.at(4));

        // restore
        painter->setRenderHint( QPainter::SmoothPixmapTransform, oldHint );

    }

}
