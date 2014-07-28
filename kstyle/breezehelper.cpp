/*
 * Copyright 2009-2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * Copyright 2008 Long Huynh Huu <long.upcase@googlemail.com>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright 2007 Fredrik H?glund <fredrik@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "breezehelper.h"

#include "breezemetrics.h"

#include <QPainter>

namespace Breeze
{
    //____________________________________________________________________
    Helper::Helper( KSharedConfig::Ptr config ):
        _config( config )
    {}

    //____________________________________________________________________
    KSharedConfigPtr Helper::config() const
    { return _config; }

    //____________________________________________________________________
    void Helper::loadConfig()
    {
        _viewFocusBrush = KStatefulBrush( KColorScheme::View, KColorScheme::FocusColor, _config );
        _viewHoverBrush = KStatefulBrush( KColorScheme::View, KColorScheme::HoverColor, _config );
        _viewNegativeTextBrush = KStatefulBrush( KColorScheme::View, KColorScheme::NegativeText, _config );
    }

    //______________________________________________________________________________
    void Helper::invalidateCaches( void )
    {
        _scrollBarHandleCache.clear();
    }

    //____________________________________________________________________
    void Helper::setMaxCacheSize( int value )
    {
        _scrollBarHandleCache.setMaxCost( value );
    }

    //____________________________________________________________________
    QColor Helper::alphaColor( QColor color, qreal alpha )
    {
        if( alpha >= 0 && alpha < 1.0 )
        { color.setAlphaF( alpha*color.alphaF() ); }
        return color;
    }

    //________________________________________________________________________________________________________
    TileSet *Helper::scrollBarHandle( const QColor& color, const QColor& glow)
    {

        const quint64 key( ( colorKey(color) << 32 ) | colorKey(glow) );
        TileSet *tileSet = _scrollBarHandleCache.object( key );

        if ( !tileSet )
        {

            QPixmap pm( Metrics::ScrollBar_SliderWidth, Metrics::ScrollBar_SliderWidth );
            pm.fill( Qt::transparent );

            QPainter p( &pm );
            p.setRenderHints( QPainter::Antialiasing );

            // content
            if( color.isValid() )
            {
                p.setPen( Qt::NoPen );
                p.setBrush( color );
                // p.drawEllipse( QRectF( 0.5, 0.5, Metrics::ScrollBar_SliderWidth-1, Metrics::ScrollBar_SliderWidth-1 ) );
                p.drawEllipse( QRectF( 0, 0, Metrics::ScrollBar_SliderWidth, Metrics::ScrollBar_SliderWidth ) );
            }

            // border
            if( glow.isValid() )
            {
                p.setPen( QPen( glow, 2 ) );
                p.setBrush( Qt::NoBrush );
                p.drawEllipse( QRectF( 1, 1, Metrics::ScrollBar_SliderWidth-2, Metrics::ScrollBar_SliderWidth-2 ) );
            }

            p.end();

            // create tileset and return
            tileSet = new TileSet( pm, Metrics::ScrollBar_SliderWidth/2, Metrics::ScrollBar_SliderWidth/2, 1, 1 );
            _scrollBarHandleCache.insert( key, tileSet );

        }

        return tileSet;
    }

    //________________________________________________________________________________________________________
    TileSet *Helper::scrollBarHole( const QColor& color )
    {
        // in the current implementation, holes and handles are rendered with the same code
        return scrollBarHandle( color, QColor() );
    }

}
