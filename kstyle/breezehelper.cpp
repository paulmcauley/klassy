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

#include <KColorUtils>
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
    {}

    //____________________________________________________________________
    void Helper::setMaxCacheSize( int )
    {}

    //______________________________________________________________________________
    QPalette Helper::mergePalettes( const QPalette& source, qreal ratio ) const
    {

        QPalette out( source );
        out.setColor( QPalette::Background, KColorUtils::mix( source.color( QPalette::Active, QPalette::Background ), source.color( QPalette::Disabled, QPalette::Background ), 1.0-ratio ) );
        out.setColor( QPalette::Highlight, KColorUtils::mix( source.color( QPalette::Active, QPalette::Highlight ), source.color( QPalette::Disabled, QPalette::Highlight ), 1.0-ratio ) );
        out.setColor( QPalette::WindowText, KColorUtils::mix( source.color( QPalette::Active, QPalette::WindowText ), source.color( QPalette::Disabled, QPalette::WindowText ), 1.0-ratio ) );
        out.setColor( QPalette::ButtonText, KColorUtils::mix( source.color( QPalette::Active, QPalette::ButtonText ), source.color( QPalette::Disabled, QPalette::ButtonText ), 1.0-ratio ) );
        out.setColor( QPalette::Text, KColorUtils::mix( source.color( QPalette::Active, QPalette::Text ), source.color( QPalette::Disabled, QPalette::Text ), 1.0-ratio ) );
        out.setColor( QPalette::Button, KColorUtils::mix( source.color( QPalette::Active, QPalette::Button ), source.color( QPalette::Disabled, QPalette::Button ), 1.0-ratio ) );
        return out;
    }

    //____________________________________________________________________
    QColor Helper::alphaColor( QColor color, qreal alpha )
    {
        if( alpha >= 0 && alpha < 1.0 )
        { color.setAlphaF( alpha*color.alphaF() ); }
        return color;
    }


    //______________________________________________________________________________
    void Helper::renderFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline, bool focus ) const
    {

        painter->setRenderHint( QPainter::Antialiasing );

        const QRectF baseRect( rect );

        if( color.isValid() )
        {
            // content
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRoundedRect( baseRect.adjusted( 1, 1, -1, -1 ), 2.5, 2.5 );
        }

        if( outline.isValid() )
        {

            // outline
            if( focus )
            {

                painter->setPen( QPen( outline, 2 ) );
                painter->setBrush( Qt::NoBrush );
                painter->drawRoundedRect( baseRect.adjusted( 1, 1, -1, -1 ), 1.5, 1.5 );

            } else {

                painter->setPen( QPen( outline, 1 ) );
                painter->setBrush( Qt::NoBrush );
                painter->drawRoundedRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ), 2, 2 );

            }
        }

    }

    //______________________________________________________________________________
    void Helper::renderButtonSlab(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline, const QColor& shadow,
        bool focus, bool sunken ) const
    {
        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF baseRect( rect );

        // shadow
        if( !sunken )
        {
            painter->setPen( QPen( shadow, 2 ) );
            painter->setBrush( Qt::NoBrush );
            const QRectF shadowRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ).translated( 0, 0.5 ) );
            painter->drawRoundedRect( shadowRect, 2.5, 2.5 );
        }

        // content
        {

            painter->setPen( Qt::NoPen );

            const QRectF contentRect( baseRect.adjusted( 1, 1, -1, -1 ) );
            QLinearGradient gradient( contentRect.topLeft(), contentRect.bottomLeft() );
            gradient.setColorAt( 0, color.lighter( focus ? 103:101 ) );
            gradient.setColorAt( 1, color.darker( focus ? 110:103 ) );
            painter->setBrush( gradient );

            painter->drawRoundedRect( contentRect, 3, 3 );

        }

        // outline
        if( outline.isValid() )
        {
            painter->setPen( QPen( outline, 1 ) );
            painter->setBrush( Qt::NoBrush );
            const QRectF outlineRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ) );
            painter->drawRoundedRect( outlineRect, 2.5, 2.5 );

        }

    }

    //______________________________________________________________________________
    void Helper::renderCheckBox(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& shadow,
        bool sunken, CheckBoxState state ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF baseRect( rect );

        // shadow
        if( !sunken )
        {

            painter->setPen( QPen( shadow, 2 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF shadowRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ).translated( 0, 0.5 ) );
            painter->drawRoundedRect( shadowRect, 2, 2 );

        }

        // content
        {

            painter->setPen( QPen( color, 2 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF contentRect( baseRect.adjusted( 2, 2, -2, -2 ) );
            painter->drawRoundedRect( contentRect, 2, 2 );

        }

        // mark
        if( state == CheckOn )
        {

            painter->setBrush( color );
            painter->setPen( Qt::NoPen );

            const QRectF markerRect( baseRect.adjusted( 5, 5, -5, -5 ) );
            painter->drawRect( markerRect );

        } else if( state == CheckPartial ) {

            QPen pen( color, 2 );
            pen.setJoinStyle( Qt::MiterJoin );
            painter->setPen( pen );

            const QRectF markerRect( baseRect.adjusted( 6, 6, -6, -6 ) );
            painter->drawRect( markerRect );

            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->setRenderHint( QPainter::Antialiasing, false );

            QPainterPath path;
            path.moveTo( 5, 5 );
            path.lineTo( qreal( Metrics::CheckBox_Size ) -6, 5 );
            path.lineTo( 5, qreal( Metrics::CheckBox_Size ) - 6 );
            painter->drawPath( path );

        }

    }

    //______________________________________________________________________________
    void Helper::renderRadioButton(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& shadow,
        bool sunken, bool checked ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        QRectF baseRect( rect );

        // shadow
        if( !sunken )
        {

            painter->setPen( QPen( shadow, 2 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF shadowRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ).translated( 0, 0.5 ) );
            painter->drawEllipse( shadowRect );

        }

        // content
        {

            painter->setPen( QPen( color, 2 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF contentRect( baseRect.adjusted( 2, 2, -2, -2 ) );
            painter->drawEllipse( contentRect );

        }

        // mark
        if( checked )
        {

            painter->setBrush( color );
            painter->setPen( Qt::NoPen );

            const QRectF markerRect( baseRect.adjusted( 5, 5, -5, -5 ) );
            painter->drawEllipse( markerRect );

        }

    }

    //______________________________________________________________________________
    void Helper::renderSliderHole(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );
        const qreal radius( 0.5*Metrics::Slider_Thickness );

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRoundedRect( baseRect, radius, radius );
        }

        return;

    }

    //______________________________________________________________________________
    void Helper::renderSliderHandle(
        QPainter* painter, const QRect& rect,
        const QColor& color,
        const QColor& outline,
        const QColor& shadow,
        bool focus,
        bool sunken ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        QRectF baseRect( rect );

        // shadow
        if( !sunken )
        {

            painter->setPen( QPen( shadow, 2 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF shadowRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ).translated( 0, 0.5 ) );
            painter->drawEllipse( shadowRect );

        }

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );

            const QRectF contentRect( baseRect.adjusted( 1, 1, -1, -1 ) );
            painter->drawEllipse( contentRect );

        }

        // outline
        if( outline.isValid() )
        {
            painter->setBrush( Qt::NoBrush );
            QRectF outlineRect;
            if( focus )
            {
                painter->setPen( QPen( outline, 2 ) );
                outlineRect = baseRect.adjusted( 2, 2, -2, -2 );
            } else {
                painter->setPen( QPen( outline, 1 ) );
                outlineRect = baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 );
            }

            painter->drawEllipse( outlineRect );

        }

    }

    //______________________________________________________________________________
    void Helper::renderProgressBarHole(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );
        const qreal radius( 0.5*Metrics::ProgressBar_Thickness );

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRoundedRect( baseRect, radius, radius );
        }

        return;

    }


    //______________________________________________________________________________
    void Helper::renderProgressBarBusyContents(
        QPainter* painter, const QRect& rect,
        const QColor& first,
        const QColor& second,
        bool horizontal,
        int progress
        ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );
        const qreal radius( 0.5*Metrics::ProgressBar_Thickness );

        // setup brush
        QPixmap pixmap( horizontal ? 2*Metrics::ProgressBar_BusyIndicatorSize : 1, horizontal ? 1:2*Metrics::ProgressBar_BusyIndicatorSize );
        pixmap.fill( second );
        if( horizontal )
        {

            QPainter painter( &pixmap );
            painter.setBrush( first );
            painter.setPen( Qt::NoPen );

            progress %= 2*Metrics::ProgressBar_BusyIndicatorSize;
            painter.drawRect( QRect( 0, 0, Metrics::ProgressBar_BusyIndicatorSize, 1 ).translated( progress, 0 ) );

            if( progress > Metrics::ProgressBar_BusyIndicatorSize )
            { painter.drawRect( QRect( 0, 0, Metrics::ProgressBar_BusyIndicatorSize, 1 ).translated( progress - 2*Metrics::ProgressBar_BusyIndicatorSize, 0 ) ); }

        } else {

            QPainter painter( &pixmap );
            painter.setBrush( first );
            painter.setPen( Qt::NoPen );
            painter.drawRect( QRect( 0, 0, 1, Metrics::ProgressBar_BusyIndicatorSize ).translated( 0, progress % Metrics::ProgressBar_BusyIndicatorSize ) );

        }

        painter->setPen( Qt::NoPen );
        painter->setBrush( pixmap );
        painter->drawRoundedRect( baseRect, radius, radius );

        return;

    }

    //______________________________________________________________________________
    void Helper::renderScrollBarHandle(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );
        const qreal radius( 0.5*Metrics::ScrollBar_SliderWidth );

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRoundedRect( baseRect, radius, radius );
        }


        // border
        if( outline.isValid() )
        {
            painter->setPen( QPen( outline, 2 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF outlineRect( baseRect.adjusted( 1, 1, -1, -1 ) );
            painter->drawRoundedRect( outlineRect, radius - 0.5, radius - 0.5 );
        }

        return;

    }

}
