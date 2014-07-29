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
        _radioButtonCache.clear();
        _checkBoxCache.clear();
        _scrollBarHandleCache.clear();
    }

    //____________________________________________________________________
    void Helper::setMaxCacheSize( int value )
    {
        _radioButtonCache.setMaxCost( value );
        _checkBoxCache.setMaxCost( value );
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
    QPixmap* Helper::checkBox( const QColor& color, const QColor& shadow, bool sunken, CheckBoxState state )
    {

        // shadow does not enter the key as it is supposed to always be the same color
        const quint64 key( ( colorKey(color) << 32 ) | (sunken<<2) | state );
        QPixmap* pixmap( _checkBoxCache.object( key ) );
        if( !pixmap )
        {

            pixmap = new QPixmap( Metrics::RadioButton_Size, Metrics::CheckBox_Size );
            pixmap->fill( Qt::transparent );

            QPainter painter( pixmap );
            painter.setRenderHints( QPainter::Antialiasing );

            const QRectF baseRect( 0, 0, Metrics::CheckBox_Size, Metrics::CheckBox_Size );

            // shadow
            if( !sunken )
            {

                painter.setPen( QPen( shadow, 2 ) );
                painter.setBrush( Qt::NoBrush );

                const QRectF shadowRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ).translated( 0, 0.5 ) );
                painter.drawRoundedRect( shadowRect, 2, 2 );

            }

            // content
            {

                painter.setPen( QPen( color, 2 ) );
                painter.setBrush( Qt::NoBrush );

                const QRectF contentRect( baseRect.adjusted( 2, 2, -2, -2 ) );
                painter.drawRoundedRect( contentRect, 2, 2 );

            }

            // mark
            if( state == CheckOn )
            {

                painter.setBrush( color );
                painter.setPen( Qt::NoPen );

                const QRectF markerRect( baseRect.adjusted( 5, 5, -5, -5 ) );
                painter.drawRect( markerRect );

            } else if( state == CheckPartial ) {

                QPen pen( color, 2 );
                pen.setJoinStyle( Qt::MiterJoin );
                painter.setPen( pen );

                const QRectF markerRect( baseRect.adjusted( 6, 6, -6, -6 ) );
                painter.drawRect( markerRect );

                painter.setPen( Qt::NoPen );
                painter.setBrush( color );
                painter.setRenderHint( QPainter::Antialiasing, false );

                QPainterPath path;
                path.moveTo( 5, 5 );
                path.lineTo( qreal( Metrics::CheckBox_Size ) -6, 5 );
                path.lineTo( 5, qreal( Metrics::CheckBox_Size ) - 6 );
                painter.drawPath( path );

            }

            _checkBoxCache.insert( key, pixmap );

        }

        return pixmap;

    }

    //________________________________________________________________________________________________________
    QPixmap* Helper::radioButton( const QColor& color, const QColor& shadow, bool sunken, bool checked )
    {

        // shadow does not enter the key as it is supposed to always be the same color
        const quint64 key( ( colorKey(color) << 32 ) | (sunken<<1) | checked );
        QPixmap* pixmap( _radioButtonCache.object( key ) );
        if( !pixmap )
        {

            pixmap = new QPixmap( Metrics::RadioButton_Size, Metrics::RadioButton_Size );
            pixmap->fill( Qt::transparent );

            QPainter painter( pixmap );
            painter.setRenderHints( QPainter::Antialiasing );

            const QRectF baseRect( 0, 0, Metrics::RadioButton_Size, Metrics::RadioButton_Size );

            // shadow
            if( !sunken )
            {

                painter.setPen( QPen( shadow, 2 ) );
                painter.setBrush( Qt::NoBrush );

                const QRectF shadowRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ).translated( 0, 0.5 ) );
                painter.drawEllipse( shadowRect );

            }

            // content
            {

                painter.setPen( QPen( color, 2 ) );
                painter.setBrush( Qt::NoBrush );

                const QRectF contentRect( baseRect.adjusted( 2, 2, -2, -2 ) );
                painter.drawEllipse( contentRect );

            }

            // mark
            if( checked )
            {

                painter.setBrush( color );
                painter.setPen( Qt::NoPen );

                const QRectF markerRect( baseRect.adjusted( 5, 5, -5, -5 ) );
                painter.drawEllipse( markerRect );

            }

            _radioButtonCache.insert( key, pixmap );

        }

        return pixmap;

    }

    //________________________________________________________________________________________________________
    TileSet* Helper::scrollBarHandle( const QColor& color, const QColor& glow)
    {

        const quint64 key( ( colorKey(color) << 32 ) | colorKey(glow) );
        TileSet* tileSet( _scrollBarHandleCache.object( key ) );

        if( !tileSet )
        {

            QPixmap pixmap( Metrics::ScrollBar_SliderWidth, Metrics::ScrollBar_SliderWidth );
            pixmap.fill( Qt::transparent );

            QPainter painter( &pixmap );
            painter.setRenderHints( QPainter::Antialiasing );

            // content
            if( color.isValid() )
            {
                painter.setPen( Qt::NoPen );
                painter.setBrush( color );
                painter.drawEllipse( QRectF( 0, 0, Metrics::ScrollBar_SliderWidth, Metrics::ScrollBar_SliderWidth ) );
            }

            // border
            if( glow.isValid() )
            {
                painter.setPen( QPen( glow, 2 ) );
                painter.setBrush( Qt::NoBrush );
                painter.drawEllipse( QRectF( 1, 1, Metrics::ScrollBar_SliderWidth-2, Metrics::ScrollBar_SliderWidth-2 ) );
            }

            painter.end();

            // create tileset and return
            tileSet = new TileSet( pixmap, Metrics::ScrollBar_SliderWidth/2, Metrics::ScrollBar_SliderWidth/2, 1, 1 );
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
