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

#include "breezehelper.h"

#include "breeze.h"
#include "breezestyleconfigdata.h"

#include <KColorUtils>
#include <KWindowSystem>

#include <QApplication>
#include <QPainter>

#if BREEZE_HAVE_X11 && QT_VERSION < 0x050000
#include <X11/Xlib-xcb.h>
#endif

#include <algorithm>

namespace Breeze
{

    //* contrast for arrow and treeline rendering
    static const qreal arrowShade = 0.15;

    //____________________________________________________________________
    Helper::Helper( KSharedConfig::Ptr config ):
        _config( std::move( config ) )
    { init(); }

    //____________________________________________________________________
    #if BREEZE_USE_KDE4
    Helper::Helper( const QByteArray& name ):
        _componentData( name, nullptr, KComponentData::SkipMainComponentRegistration ),
        _config( _componentData.config() )
    { init(); }
    #endif

    //____________________________________________________________________
    KSharedConfig::Ptr Helper::config() const
    { return _config; }

    //____________________________________________________________________
    void Helper::loadConfig()
    {
        _viewFocusBrush = KStatefulBrush( KColorScheme::View, KColorScheme::FocusColor );
        _viewHoverBrush = KStatefulBrush( KColorScheme::View, KColorScheme::HoverColor );
        _viewNegativeTextBrush = KStatefulBrush( KColorScheme::View, KColorScheme::NegativeText );

        const QPalette palette( QApplication::palette() );
        const KConfigGroup group( _config->group( "WM" ) );
        _activeTitleBarColor = group.readEntry( "activeBackground", palette.color( QPalette::Active, QPalette::Highlight ) );
        _activeTitleBarTextColor = group.readEntry( "activeForeground", palette.color( QPalette::Active, QPalette::HighlightedText ) );
        _inactiveTitleBarColor = group.readEntry( "inactiveBackground", palette.color( QPalette::Disabled, QPalette::Highlight ) );
        _inactiveTitleBarTextColor = group.readEntry( "inactiveForeground", palette.color( QPalette::Disabled, QPalette::HighlightedText ) );
    }

    //____________________________________________________________________
    QColor Helper::frameOutlineColor( const QPalette& palette, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode mode ) const
    {

        QColor outline( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ) );

        // focus takes precedence over hover
        if( mode == AnimationFocus )
        {

            const QColor focus( focusColor( palette ) );
            const QColor hover( hoverColor( palette ) );

            if( mouseOver ) outline = KColorUtils::mix( hover, focus, opacity );
            else outline = KColorUtils::mix( outline, focus, opacity );

        } else if( hasFocus ) {

            outline = focusColor( palette );

        } else if( mode == AnimationHover ) {

            const QColor hover( hoverColor( palette ) );
            outline = KColorUtils::mix( outline, hover, opacity );

        } else if( mouseOver ) {

            outline = hoverColor( palette );

        }

        return outline;

    }

    //____________________________________________________________________
    QColor Helper::focusOutlineColor( const QPalette& palette ) const
    { return KColorUtils::mix( focusColor( palette ), palette.color( QPalette::WindowText ), 0.15 ); }

    //____________________________________________________________________
    QColor Helper::hoverOutlineColor( const QPalette& palette ) const
    { return KColorUtils::mix( hoverColor( palette ), palette.color( QPalette::WindowText ), 0.15 ); }

    //____________________________________________________________________
    QColor Helper::buttonFocusOutlineColor( const QPalette& palette ) const
    { return KColorUtils::mix( focusColor( palette ), palette.color( QPalette::ButtonText ), 0.15 ); }

    //____________________________________________________________________
    QColor Helper::buttonHoverOutlineColor( const QPalette& palette ) const
    { return KColorUtils::mix( hoverColor( palette ), palette.color( QPalette::ButtonText ), 0.15 ); }

    //____________________________________________________________________
    QColor Helper::sidePanelOutlineColor( const QPalette& palette, bool hasFocus, qreal opacity, AnimationMode mode ) const
    {

        QColor outline( palette.color( QPalette::Inactive, QPalette::Highlight ) );
        const QColor &focus = palette.color( QPalette::Active, QPalette::Highlight );

        if( mode == AnimationFocus )
        {

            outline = KColorUtils::mix( outline, focus, opacity );

        } else if( hasFocus ) {

            outline = focus;

        }

        return outline;

    }

    //____________________________________________________________________
    QColor Helper::frameBackgroundColor( const QPalette& palette, QPalette::ColorGroup group ) const
    { return KColorUtils::mix( palette.color( group, QPalette::Window ), palette.color( group, QPalette::Base ), 0.3 ); }

    //____________________________________________________________________
    QColor Helper::arrowColor( const QPalette& palette, QPalette::ColorGroup group, QPalette::ColorRole role ) const
    {
        switch( role )
        {
            case QPalette::Text: return KColorUtils::mix( palette.color( group, QPalette::Text ), palette.color( group, QPalette::Base ), arrowShade );
            case QPalette::WindowText: return KColorUtils::mix( palette.color( group, QPalette::WindowText ), palette.color( group, QPalette::Window ), arrowShade );
            case QPalette::ButtonText: return KColorUtils::mix( palette.color( group, QPalette::ButtonText ), palette.color( group, QPalette::Button ), arrowShade );
            default: return palette.color( group, role );
        }

    }

    //____________________________________________________________________
    QColor Helper::arrowColor( const QPalette& palette, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode mode ) const
    {

        QColor outline( arrowColor( palette, QPalette::WindowText ) );
        if( mode == AnimationHover )
        {

            const QColor focus( focusColor( palette ) );
            const QColor hover( hoverColor( palette ) );
            if( hasFocus ) outline = KColorUtils::mix( focus, hover, opacity );
            else outline = KColorUtils::mix( outline, hover, opacity );

        } else if( mouseOver ) {

            outline = hoverColor( palette );

        } else if( mode == AnimationFocus ) {

            const QColor focus( focusColor( palette ) );
            outline = KColorUtils::mix( outline, focus, opacity );

        } else if( hasFocus ) {

            outline = focusColor( palette );

        }

        return outline;

    }

    //____________________________________________________________________
    QColor Helper::buttonOutlineColor( const QPalette& palette, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode mode ) const
    {

        QColor outline( KColorUtils::mix( palette.color( QPalette::Button ), palette.color( QPalette::ButtonText ), 0.3 ) );
        if( mode == AnimationHover )
        {

            if( hasFocus )
            {
                const QColor focus( buttonFocusOutlineColor( palette ) );
                const QColor hover( buttonHoverOutlineColor( palette ) );
                outline = KColorUtils::mix( focus, hover, opacity );

            } else {

                const QColor hover( hoverColor( palette ) );
                outline = KColorUtils::mix( outline, hover, opacity );

            }

        } else if( mouseOver ) {

            if( hasFocus ) outline = buttonHoverOutlineColor( palette );
            else outline = hoverColor( palette );

        } else if( mode == AnimationFocus ) {

            const QColor focus( buttonFocusOutlineColor( palette ) );
            outline = KColorUtils::mix( outline, focus, opacity );

        } else if( hasFocus ) {

            outline = buttonFocusOutlineColor( palette );

        }

        return outline;

    }

    //____________________________________________________________________
    QColor Helper::buttonBackgroundColor( const QPalette& palette, bool mouseOver, bool hasFocus, bool sunken, qreal opacity, AnimationMode mode ) const
    {

        QColor background( sunken ?
            KColorUtils::mix( palette.color( QPalette::Button ), palette.color( QPalette::ButtonText ), 0.2 ):
            palette.color( QPalette::Button ) );

        if( mode == AnimationHover )
        {

            const QColor focus( focusColor( palette ) );
            const QColor hover( hoverColor( palette ) );
            if( hasFocus ) background = KColorUtils::mix( focus, hover, opacity );

        } else if( mouseOver && hasFocus ) {

            background = hoverColor( palette );

        } else if( mode == AnimationFocus ) {

            const QColor focus( focusColor( palette ) );
            background = KColorUtils::mix( background, focus, opacity );

        } else if( hasFocus ) {

            background = focusColor( palette );

        }

        return background;

    }

    //____________________________________________________________________
    QColor Helper::toolButtonColor( const QPalette& palette, bool mouseOver, bool hasFocus, bool sunken, qreal opacity, AnimationMode mode ) const
    {

        QColor outline;
        const QColor hoverColor( this->hoverColor( palette ) );
        const QColor focusColor( this->focusColor( palette ) );
        const QColor sunkenColor = alphaColor( palette.color( QPalette::WindowText ), 0.2 );

        // hover takes precedence over focus
        if( mode == AnimationHover )
        {

            if( hasFocus ) outline = KColorUtils::mix( focusColor, hoverColor, opacity );
            else if( sunken ) outline = sunkenColor;
            else outline = alphaColor( hoverColor, opacity );

        } else if( mouseOver ) {

            outline = hoverColor;

        } else if( mode == AnimationFocus ) {

            if( sunken ) outline = KColorUtils::mix( sunkenColor, focusColor, opacity );
            else outline = alphaColor( focusColor, opacity );

        } else if( hasFocus ) {

            outline = focusColor;

        } else if( sunken ) {

            outline = sunkenColor;

        }

        return outline;

    }

    //____________________________________________________________________
    QColor Helper::sliderOutlineColor( const QPalette& palette, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode mode ) const
    {

        QColor outline( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.4 ) );

        // hover takes precedence over focus
        if( mode == AnimationHover )
        {

            const QColor hover( hoverColor( palette ) );
            const QColor focus( focusColor( palette ) );
            if( hasFocus ) outline = KColorUtils::mix( focus, hover, opacity );
            else outline = KColorUtils::mix( outline, hover, opacity );

        } else if( mouseOver ) {

            outline = hoverColor( palette );

        } else if( mode == AnimationFocus ) {

            const QColor focus( focusColor( palette ) );
            outline = KColorUtils::mix( outline, focus, opacity );

        } else if( hasFocus ) {

            outline = focusColor( palette );

        }

        return outline;

    }

    //____________________________________________________________________
    QColor Helper::scrollBarHandleColor( const QPalette& palette, bool mouseOver, bool hasFocus, qreal opacity, AnimationMode mode ) const
    {

        QColor color( alphaColor( palette.color( QPalette::WindowText ), 0.5 ) );

        // hover takes precedence over focus
        if( mode == AnimationHover )
        {

            const QColor hover( hoverColor( palette ) );
            const QColor focus( focusColor( palette ) );
            if( hasFocus ) color = KColorUtils::mix( focus, hover, opacity );
            else color = KColorUtils::mix( color, hover, opacity );

        } else if( mouseOver ) {

            color = hoverColor( palette );

        } else if( mode == AnimationFocus ) {

            const QColor focus( focusColor( palette ) );
            color = KColorUtils::mix( color, focus, opacity );

        } else if( hasFocus ) {

            color = focusColor( palette );

        }

        return color;

    }

    //______________________________________________________________________________
    QColor Helper::checkBoxIndicatorColor( const QPalette& palette, bool mouseOver, bool active, qreal opacity, AnimationMode mode ) const
    {

        QColor color( KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.6 ) );
        if( mode == AnimationHover )
        {

            const QColor focus( focusColor( palette ) );
            const QColor hover( hoverColor( palette ) );
            if( active ) color =  KColorUtils::mix( focus, hover, opacity );
            else color = KColorUtils::mix( color, hover, opacity );

        } else if( mouseOver ) {

            color = hoverColor( palette );

        } else if( active ) {

            color = focusColor( palette );

        }

        return color;

    }

    //______________________________________________________________________________
    QColor Helper::separatorColor( const QPalette& palette ) const
    { return KColorUtils::mix( palette.color( QPalette::Window ), palette.color( QPalette::WindowText ), 0.25 ); }

    //______________________________________________________________________________
    QPalette Helper::disabledPalette( const QPalette& source, qreal ratio ) const
    {

        QPalette copy( source );

        const QList<QPalette::ColorRole> roles = { QPalette::Background, QPalette::Highlight, QPalette::WindowText, QPalette::ButtonText, QPalette::Text, QPalette::Button };
        foreach( const QPalette::ColorRole& role, roles )
        { copy.setColor( role, KColorUtils::mix( source.color( QPalette::Active, role ), source.color( QPalette::Disabled, role ), 1.0-ratio ) ); }

        return copy;
    }

    //____________________________________________________________________
    QColor Helper::alphaColor( QColor color, qreal alpha ) const
    {
        if( alpha >= 0 && alpha < 1.0 )
        { color.setAlphaF( alpha*color.alphaF() ); }
        return color;
    }

    //______________________________________________________________________________
    void Helper::renderDebugFrame( QPainter* painter, const QRect& rect ) const
    {
        painter->save();
        painter->setRenderHints( QPainter::Antialiasing );
        painter->setBrush( Qt::NoBrush );
        painter->setPen( Qt::red );
        painter->drawRect( QRectF( rect ).adjusted( 0.5, 0.5, -0.5, -0.5 ) );
        painter->restore();
    }

    //______________________________________________________________________________
    void Helper::renderFocusRect( QPainter* painter, const QRect& rect, const QColor& color, const QColor& outline, Sides sides ) const
    {
        if( !color.isValid() ) return;

        painter->save();
        painter->setRenderHints( QPainter::Antialiasing );
        painter->setBrush( color );

        if( !( outline.isValid() && sides ) )
        {

            painter->setPen( Qt::NoPen );
            painter->drawRect( rect );

        } else {

            painter->setClipRect( rect );

            QRectF copy( rect );
            copy.adjust( 0.5, 0.5, -0.5, -0.5 );

            const qreal radius( frameRadius( -1.0 ) );
            if( !(sides&SideTop) ) copy.adjust( 0, -radius, 0, 0 );
            if( !(sides&SideBottom) ) copy.adjust( 0, 0, 0, radius );
            if( !(sides&SideLeft) ) copy.adjust( -radius, 0, 0, 0 );
            if( !(sides&SideRight) ) copy.adjust( 0, 0, radius, 0 );

            painter->setPen( outline );
            // painter->setBrush( Qt::NoBrush );
            painter->drawRoundedRect( copy, radius, radius );

        }

        painter->restore();
   }

    //______________________________________________________________________________
    void Helper::renderFocusLine( QPainter* painter, const QRect& rect, const QColor& color ) const
    {
        if( !color.isValid() ) return;

        painter->save();
        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setBrush( Qt::NoBrush );
        painter->setPen( color );

        painter->translate( 0, 2 );
        painter->drawLine( rect.bottomLeft(), rect.bottomRight() );
        painter->restore();
    }

    //______________________________________________________________________________
    void Helper::renderFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline ) const
    {

        painter->setRenderHint( QPainter::Antialiasing );

        QRectF frameRect( rect.adjusted( 1, 1, -1, -1 ) );
        qreal radius( frameRadius() );

        // set pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );
            radius = qMax( radius - 1, qreal( 0.0 ) );

        } else {

            painter->setPen( Qt::NoPen );

        }

        // set brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        // render
        painter->drawRoundedRect( frameRect, radius, radius );

    }

    //______________________________________________________________________________
    void Helper::renderSidePanelFrame( QPainter* painter, const QRect& rect, const QColor& outline, Side side ) const
    {

        // check color
        if( !outline.isValid() ) return;

        // adjust rect
        QRectF frameRect( rect );
        frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing );
        painter->setPen( outline );

        // render
        switch( side )
        {
            default:
            case SideLeft:
            painter->drawLine( frameRect.topRight(), frameRect.bottomRight() );
            break;

            case SideTop:
            painter->drawLine( frameRect.topLeft(), frameRect.topRight() );
            break;

            case SideRight:
            painter->drawLine( frameRect.topLeft(), frameRect.bottomLeft() );
            break;

            case SideBottom:
            painter->drawLine( frameRect.bottomLeft(), frameRect.bottomRight() );
            break;

            case AllSides:
            {
                const qreal radius( frameRadius( -1.0 ) );
                painter->drawRoundedRect( frameRect, radius, radius );
                break;
            }

        }

    }

    //______________________________________________________________________________
    void Helper::renderMenuFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline, bool roundCorners ) const
    {


        // set brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        if( roundCorners )
        {

            painter->setRenderHint( QPainter::Antialiasing );
            QRectF frameRect( rect );
            qreal radius( frameRadius() );

            // set pen
            if( outline.isValid() )
            {

                painter->setPen( outline );
                frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );
                radius = qMax( radius - 1, qreal( 0.0 ) );

            } else painter->setPen( Qt::NoPen );

            // render
            painter->drawRoundedRect( frameRect, radius, radius );

        } else {

            painter->setRenderHint( QPainter::Antialiasing, false );
            QRect frameRect( rect );
            if( outline.isValid() )
            {

                painter->setPen( outline );
                frameRect.adjust( 0, 0, -1, -1 );

            } else painter->setPen( Qt::NoPen );

            painter->drawRect( frameRect );

        }

    }

    //______________________________________________________________________________
    void Helper::renderButtonFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline, const QColor& shadow,
        bool hasFocus, bool sunken ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 1, 1, -1, -1 );
        qreal radius( frameRadius() );

        // shadow
        if( sunken ) {

            frameRect.translate( 1, 1 );

        } else if( shadow.isValid() ) {

            const qreal shadowRadius = qMax( radius - 1, qreal( 0.0 ) );
            painter->setPen( QPen( shadow, 2 ) );
            painter->setBrush( Qt::NoBrush );
            painter->drawRoundedRect( shadowRect( frameRect ), shadowRadius, shadowRadius );

        }

        if( outline.isValid() )
        {

            QLinearGradient gradient( frameRect.topLeft(), frameRect.bottomLeft() );
            gradient.setColorAt( 0, outline.lighter( hasFocus ? 103:101 ) );
            gradient.setColorAt( 1, outline.darker( hasFocus ? 110:103 ) );
            painter->setPen( QPen( QBrush( gradient ), 1.0 ) );

            frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );
            radius = qMax( radius - 1, qreal( 0.0 ) );

        } else painter->setPen( Qt::NoPen );

        // content
        if( color.isValid() )
        {

            QLinearGradient gradient( frameRect.topLeft(), frameRect.bottomLeft() );
            gradient.setColorAt( 0, color.lighter( hasFocus ? 103:101 ) );
            gradient.setColorAt( 1, color.darker( hasFocus ? 110:103 ) );
            painter->setBrush( gradient );

        } else painter->setBrush( Qt::NoBrush );

        // render
        painter->drawRoundedRect( frameRect, radius, radius );

    }

    //______________________________________________________________________________
    void Helper::renderToolButtonFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, bool sunken ) const
    {

        // do nothing for invalid color
        if( !color.isValid() ) return;

        // setup painter
        painter->setRenderHints( QPainter::Antialiasing );

        const QRectF baseRect( rect );

        if( sunken )
        {

            const qreal radius( frameRadius() );

            painter->setPen( Qt::NoPen );
            painter->setBrush( color );

            const QRectF contentRect( baseRect.adjusted( 1, 1, -1, -1 ) );
            painter->drawRoundedRect( contentRect, radius, radius );

        } else {

            const qreal radius( frameRadius(-0.5) );

            painter->setPen( color );
            painter->setBrush( Qt::NoBrush );
            const QRectF outlineRect( baseRect.adjusted( 1.5, 1.5, -1.5, -1.5 ) );
            painter->drawRoundedRect( outlineRect, radius, radius );

        }

    }

    //______________________________________________________________________________
    void Helper::renderToolBoxFrame(
        QPainter* painter, const QRect& rect, int tabWidth,
        const QColor& outline ) const
    {

        if( !outline.isValid() ) return;

        // round radius
        const qreal radius( frameRadius() );
        const QSizeF cornerSize( 2*radius, 2*radius );

        // if rect - tabwidth is even, need to increase tabWidth by 1 unit
        // for anti aliasing
        if( !((rect.width() - tabWidth)%2) ) ++tabWidth;

        // adjust rect for antialiasing
        QRectF baseRect( rect );
        baseRect.adjust( 0.5, 0.5, -0.5, -0.5 );

        // create path
        QPainterPath path;
        path.moveTo( 0, baseRect.height()-1 );
        path.lineTo( ( baseRect.width() - tabWidth )/2 - radius, baseRect.height()-1 );
        path.arcTo( QRectF( QPointF( ( baseRect.width() - tabWidth )/2 - 2*radius, baseRect.height()-1 - 2*radius ), cornerSize ), 270, 90 );
        path.lineTo( ( baseRect.width() - tabWidth )/2, radius );
        path.arcTo( QRectF( QPointF( ( baseRect.width() - tabWidth )/2, 0 ), cornerSize ), 180, -90 );
        path.lineTo( ( baseRect.width() + tabWidth )/2 -1 - radius, 0 );
        path.arcTo( QRectF(  QPointF( ( baseRect.width() + tabWidth )/2  - 1 - 2*radius, 0 ), cornerSize ), 90, -90 );
        path.lineTo( ( baseRect.width() + tabWidth )/2 -1, baseRect.height()-1 - radius );
        path.arcTo( QRectF( QPointF( ( baseRect.width() + tabWidth )/2 -1, baseRect.height()-1 - 2*radius ), cornerSize ), 180, 90 );
        path.lineTo( baseRect.width()-1, baseRect.height()-1 );

        // render
        painter->setRenderHints( QPainter::Antialiasing );
        painter->setBrush( Qt::NoBrush );
        painter->setPen( outline );
        painter->translate( baseRect.topLeft() );
        painter->drawPath( path );

    }

    //______________________________________________________________________________
    void Helper::renderTabWidgetFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline, Corners corners ) const
    {

        painter->setRenderHint( QPainter::Antialiasing );

        QRectF frameRect( rect.adjusted( 1, 1, -1, -1 ) );
        qreal radius( frameRadius() );

        // set pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );
            radius = qMax( radius-1, qreal( 0.0 ) );

        } else painter->setPen( Qt::NoPen );

        // set brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        // render
        QPainterPath path( roundedPath( frameRect, corners, radius ) );
        painter->drawPath( path );

    }


    //______________________________________________________________________________
    void Helper::renderSelection(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        painter->setRenderHint( QPainter::Antialiasing );
        painter->setPen( Qt::NoPen );
        painter->setBrush( color );
        painter->drawRect( rect );

    }

    //______________________________________________________________________________
    void Helper::renderSeparator(
        QPainter* painter, const QRect& rect,
        const QColor& color, bool vertical ) const
    {

        painter->setRenderHint( QPainter::Antialiasing, false );
        painter->setBrush( Qt::NoBrush );
        painter->setPen( color );

        if( vertical )
        {

            painter->translate( rect.width()/2, 0 );
            painter->drawLine( rect.topLeft(), rect.bottomLeft() );

        } else {

            painter->translate( 0, rect.height()/2 );
            painter->drawLine( rect.topLeft(), rect.topRight() );


        }

        
    }

    //______________________________________________________________________________
    void Helper::renderCheckBoxBackground(
        QPainter* painter, const QRect& rect,
        const QColor& color, bool sunken ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect and radius
        QRectF frameRect( rect );
        frameRect.adjust( 3, 3, -3, -3 );

        if( sunken ) frameRect.translate(1, 1);

        painter->setPen( Qt::NoPen );
        painter->setBrush( color );
        painter->drawRect( frameRect );

    }

    //______________________________________________________________________________
    void Helper::renderCheckBox(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& shadow,
        bool sunken, CheckBoxState state, qreal animation ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect and radius
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );
        qreal radius( frameRadius() );

        // shadow
        if( sunken )
        {

            frameRect.translate(1, 1);

        } else {

            painter->setPen( QPen( shadow, 1 ) );
            painter->setBrush( Qt::NoBrush );

            const qreal shadowRadius( radius + 0.5 );
            painter->drawRoundedRect( shadowRect( frameRect ).adjusted( -0.5, -0.5, 0.5, 0.5 ), shadowRadius, shadowRadius );

        }

        // content
        {

            painter->setPen( QPen( color, 1 ) );
            painter->setBrush( Qt::NoBrush );

            radius = qMax( radius-1, qreal( 0.0 ) );
            const QRectF contentRect( frameRect.adjusted( 0.5, 0.5, -0.5, -0.5 ) );
            painter->drawRoundedRect( contentRect, radius, radius );

        }

        // mark
        if( state == CheckOn )
        {

            painter->setBrush( color );
            painter->setPen( Qt::NoPen );

            const QRectF markerRect( frameRect.adjusted( 3, 3, -3, -3 ) );
            painter->drawRect( markerRect );

        } else if( state == CheckPartial ) {

            QPen pen( color, 2 );
            pen.setJoinStyle( Qt::MiterJoin );
            painter->setPen( pen );

            const QRectF markerRect( frameRect.adjusted( 4, 4, -4, -4 ) );
            painter->drawRect( markerRect );

            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->setRenderHint( QPainter::Antialiasing, false );

            QPainterPath path;
            path.moveTo( markerRect.topLeft() );
            path.lineTo( markerRect.right() - 1, markerRect.top() );
            path.lineTo( markerRect.left(), markerRect.bottom()-1 );
            painter->drawPath( path );

        } else if( state == CheckAnimated ) {

            const QRectF markerRect( frameRect.adjusted( 3, 3, -3, -3 ) );
            QPainterPath path;
            path.moveTo( markerRect.topRight() );
            path.lineTo( markerRect.center() + animation*( markerRect.topLeft() - markerRect.center() ) );
            path.lineTo( markerRect.bottomLeft() );
            path.lineTo( markerRect.center() + animation*( markerRect.bottomRight() - markerRect.center() ) );
            path.closeSubpath();

            painter->setBrush( color );
            painter->setPen( Qt::NoPen );
            painter->drawPath( path );

        }

    }

    //______________________________________________________________________________
    void Helper::renderRadioButtonBackground( QPainter* painter, const QRect& rect, const QColor& color, bool sunken ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 3, 3, -3, -3 );
        if( sunken ) frameRect.translate(1, 1);

        painter->setPen( Qt::NoPen );
        painter->setBrush( color );
        painter->drawEllipse( frameRect );

    }

    //______________________________________________________________________________
    void Helper::renderRadioButton(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& shadow,
        bool sunken, RadioButtonState state, qreal animation ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );

        // shadow
        if( sunken )
        {

            frameRect.translate( 1, 1 );

        } else {

            painter->setPen( QPen( shadow, 1 ) );
            painter->setBrush( Qt::NoBrush );
            painter->drawEllipse( shadowRect( frameRect ).adjusted( -0.5, -0.5, 0.5, 0.5 ) );

        }

        // content
        {

            painter->setPen( QPen( color, 1 ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF contentRect( frameRect.adjusted( 0.5, 0.5, -0.5, -0.5 ) );
            painter->drawEllipse( contentRect );

        }

        // mark
        if( state == RadioOn )
        {

            painter->setBrush( color );
            painter->setPen( Qt::NoPen );

            const QRectF markerRect( frameRect.adjusted( 3, 3, -3, -3 ) );
            painter->drawEllipse( markerRect );

        } else if( state == RadioAnimated ) {

            painter->setBrush( color );
            painter->setPen( Qt::NoPen );
            QRectF markerRect( frameRect.adjusted( 3, 3, -3, -3 ) );

            painter->translate( markerRect.center() );
            painter->rotate( 45 );

            markerRect.setWidth( markerRect.width()*animation );
            markerRect.translate( -markerRect.center() );
            painter->drawEllipse( markerRect );

        }

    }

    //______________________________________________________________________________
    void Helper::renderSliderGroove(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );
        const qreal radius( 0.5*Metrics::Slider_GrooveThickness );

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRoundedRect( baseRect, radius, radius );
        }

        
    }

    //______________________________________________________________________________
    void Helper::renderDialGroove(
        QPainter* painter, const QRect& rect,
        const QColor& color,
        qreal first, qreal last ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );

        // content
        if( color.isValid() )
        {
            const qreal penWidth( Metrics::Slider_GrooveThickness );
            const QRectF grooveRect( rect.adjusted( penWidth/2, penWidth/2, -penWidth/2, -penWidth/2 ) );

            // setup angles
            const int angleStart( first * 180 * 16 / M_PI );
            const int angleSpan( (last - first ) * 180 * 16 / M_PI );
            
            // setup pen
            if( angleSpan != 0 )
            {
                QPen pen( color, penWidth );
                pen.setCapStyle( Qt::RoundCap );
                painter->setPen( pen );
                painter->setBrush( Qt::NoBrush );
                painter->drawArc( grooveRect, angleStart, angleSpan );
            }
        }

        
    }

    //______________________________________________________________________________
    void Helper::renderDialContents(
        QPainter* painter, const QRect& rect,
        const QColor& color,
        qreal first, qreal second ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );

        // content
        if( color.isValid() )
        {

            // setup groove rect
            const qreal penWidth( Metrics::Slider_GrooveThickness );
            const QRectF grooveRect( rect.adjusted( penWidth/2, penWidth/2, -penWidth/2, -penWidth/2 ) );

            // setup angles
            const int angleStart( first * 180 * 16 / M_PI );
            const int angleSpan( (second - first ) * 180 * 16 / M_PI );

            // setup pen
            if( angleSpan != 0 )
            {
                QPen pen( color, penWidth );
                pen.setCapStyle( Qt::RoundCap );
                painter->setPen( pen );
                painter->setBrush( Qt::NoBrush );
                painter->drawArc( grooveRect, angleStart, angleSpan );
            }

        }

        
    }

    //______________________________________________________________________________
    void Helper::renderSliderHandle(
        QPainter* painter, const QRect& rect,
        const QColor& color,
        const QColor& outline,
        const QColor& shadow,
        bool sunken ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 1, 1, -1, -1 );

        // shadow
        if( shadow.isValid() && !sunken )
        {

            painter->setPen( QPen( shadow, 2 ) );
            painter->setBrush( Qt::NoBrush );
            painter->drawEllipse( shadowRect( frameRect ) );

        }

        // set pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );

        } else painter->setPen( Qt::NoPen );

        // set brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        // render
        painter->drawEllipse( frameRect );

    }

    //______________________________________________________________________________
    void Helper::renderProgressBarGroove(
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

        
    }


    //______________________________________________________________________________
    void Helper::renderProgressBarBusyContents(
        QPainter* painter, const QRect& rect,
        const QColor& first,
        const QColor& second,
        bool horizontal,
        bool reverse,
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
            if( reverse ) progress = 2*Metrics::ProgressBar_BusyIndicatorSize - progress - 1;
            painter.drawRect( QRect( 0, 0, Metrics::ProgressBar_BusyIndicatorSize, 1 ).translated( progress, 0 ) );

            if( progress > Metrics::ProgressBar_BusyIndicatorSize )
            { painter.drawRect( QRect( 0, 0, Metrics::ProgressBar_BusyIndicatorSize, 1 ).translated( progress - 2*Metrics::ProgressBar_BusyIndicatorSize, 0 ) ); }

        } else {

            QPainter painter( &pixmap );
            painter.setBrush( first );
            painter.setPen( Qt::NoPen );

            progress %= 2*Metrics::ProgressBar_BusyIndicatorSize;
            progress = 2*Metrics::ProgressBar_BusyIndicatorSize - progress - 1;
            painter.drawRect( QRect( 0, 0, 1, Metrics::ProgressBar_BusyIndicatorSize ).translated( 0, progress ) );

            if( progress > Metrics::ProgressBar_BusyIndicatorSize )
            { painter.drawRect( QRect( 0, 0, 1, Metrics::ProgressBar_BusyIndicatorSize ).translated( 0, progress - 2*Metrics::ProgressBar_BusyIndicatorSize ) ); }

        }

        painter->setPen( Qt::NoPen );
        painter->setBrush( pixmap );
        painter->drawRoundedRect( baseRect, radius, radius );

    }

    //______________________________________________________________________________
    void Helper::renderScrollBarHandle(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );
        const qreal radius( 0.5 * std::min({baseRect.width(), baseRect.height(), (qreal)Metrics::ScrollBar_SliderWidth}) );

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRoundedRect( baseRect, radius, radius );
        }

        
    }

    //______________________________________________________________________________
    void Helper::renderTabBarTab( QPainter* painter, const QRect& rect, const QColor& color, const QColor& outline, Corners corners ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        QRectF frameRect( rect );
        qreal radius( frameRadius() );

        // pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );
            radius = qMax( radius-1, qreal( 0.0 ) );

        } else painter->setPen( Qt::NoPen );


        // brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        // render
        QPainterPath path( roundedPath( frameRect, corners, radius ) );
        painter->drawPath( path );

    }

    //______________________________________________________________________________
    void Helper::renderArrow( QPainter* painter, const QRect& rect, const QColor& color, ArrowOrientation orientation ) const
    {
        // define polygon
        QPolygonF arrow;
        switch( orientation )
        {
            case ArrowUp: arrow = QVector<QPointF>{QPointF( -4, 2 ), QPointF( 0, -2 ), QPointF( 4, 2 )}; break;
            case ArrowDown: arrow = QVector<QPointF>{QPointF( -4, -2 ), QPointF( 0, 2 ), QPointF( 4, -2 )}; break;
            case ArrowLeft: arrow = QVector<QPointF>{QPointF( 2, -4 ), QPointF( -2, 0 ), QPointF( 2, 4 )}; break;
            case ArrowRight: arrow = QVector<QPointF>{QPointF( -2, -4 ), QPointF( 2, 0 ), QPointF( -2, 4 )}; break;
            default: break;
        }

        painter->save();
        painter->setRenderHints( QPainter::Antialiasing );
        painter->translate( QRectF( rect ).center() );
        painter->setBrush( Qt::NoBrush );
        painter->setPen( QPen( color, 1.1 ) );
        painter->drawPolyline( arrow );
        painter->restore();
   }

    //______________________________________________________________________________
    void Helper::renderDecorationButton( QPainter* painter, const QRect& rect, const QColor& color, ButtonType buttonType, bool inverted ) const
    {

        painter->save();
        painter->setViewport( rect );
        painter->setWindow( 0, 0, 18, 18 );
        painter->setRenderHints( QPainter::Antialiasing );

        // initialize pen
        QPen pen;
        pen.setCapStyle( Qt::RoundCap );
        pen.setJoinStyle( Qt::MiterJoin );

        if( inverted )
        {
            // render circle
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );

            // take out the inner part
            painter->setCompositionMode( QPainter::CompositionMode_DestinationOut );
            painter->setBrush( Qt::NoBrush );
            pen.setColor( Qt::black );

        } else {

            painter->setBrush( Qt::NoBrush );
            pen.setColor( color );

        }

        pen.setCapStyle( Qt::RoundCap );
        pen.setJoinStyle( Qt::MiterJoin );
        pen.setWidthF( 1.1*qMax(1.0, 18.0/rect.width() ) );
        painter->setPen( pen );

        switch( buttonType )
        {
            case ButtonClose:
            {
                painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                painter->drawLine( 13, 5, 5, 13 );
                break;
            }

            case ButtonMaximize:
            {
                painter->drawPolyline( QVector<QPointF>{
                    QPointF( 4, 11 ),
                    QPointF( 9, 6 ),
                    QPointF( 14, 11 )});
                break;
            }

            case ButtonMinimize:
            {

                painter->drawPolyline(QVector<QPointF>{
                    QPointF( 4, 7 ),
                    QPointF( 9, 12 ),
                    QPointF( 14, 7 )} );
                break;
            }

            case ButtonRestore:
            {
                pen.setJoinStyle( Qt::RoundJoin );
                painter->setPen( pen );
                painter->drawPolygon( QVector<QPointF>{
                    QPointF( 4.5, 9 ),
                    QPointF( 9, 4.5 ),
                    QPointF( 13.5, 9 ),
                    QPointF( 9, 13.5 )});
                break;
            }

            default: break;
        }

        painter->restore();

    }

    //______________________________________________________________________________
    bool Helper::isX11()
    {
        #if BREEZE_HAVE_X11
        #if QT_VERSION >= 0x050000
        static const bool s_isX11 = KWindowSystem::isPlatformX11();
        return s_isX11;
        #else
        return true;
        #endif
        #endif

        return false;

    }

    //______________________________________________________________________________
    bool Helper::isWayland()
    {
        #if QT_VERSION >= 0x050000
        static const bool s_isWayland = KWindowSystem::isPlatformWayland();
        return s_isWayland;
        #else
        return false;
        #endif
    }

    //______________________________________________________________________________
    QRectF Helper::shadowRect( const QRectF& rect ) const
    { return rect.adjusted( 0.5, 0.5, -0.5, -0.5 ).translated( 0.5, 0.5 ); }

    //______________________________________________________________________________
    QPainterPath Helper::roundedPath( const QRectF& rect, Corners corners, qreal radius ) const
    {

        QPainterPath path;

        // simple cases
        if( corners == 0 )
        {

            path.addRect( rect );
            return path;

        }

        if( corners == AllCorners ) {

            path.addRoundedRect( rect, radius, radius );
            return path;

        }

        const QSizeF cornerSize( 2*radius, 2*radius );

        // rotate counterclockwise
        // top left corner
        if( corners & CornerTopLeft )
        {

            path.moveTo( rect.topLeft() + QPointF( radius, 0 ) );
            path.arcTo( QRectF( rect.topLeft(), cornerSize ), 90, 90 );

        } else path.moveTo( rect.topLeft() );

        // bottom left corner
        if( corners & CornerBottomLeft )
        {

            path.lineTo( rect.bottomLeft() - QPointF( 0, radius ) );
            path.arcTo( QRectF( rect.bottomLeft() - QPointF( 0, 2*radius ), cornerSize ), 180, 90 );

        } else path.lineTo( rect.bottomLeft() );

        // bottom right corner
        if( corners & CornerBottomRight )
        {

            path.lineTo( rect.bottomRight() - QPointF( radius, 0 ) );
            path.arcTo( QRectF( rect.bottomRight() - QPointF( 2*radius, 2*radius ), cornerSize ), 270, 90 );

        } else path.lineTo( rect.bottomRight() );

        // top right corner
        if( corners & CornerTopRight )
        {

            path.lineTo( rect.topRight() + QPointF( 0, radius ) );
            path.arcTo( QRectF( rect.topRight() - QPointF( 2*radius, 0 ), cornerSize ), 0, 90 );

        } else path.lineTo( rect.topRight() );

        path.closeSubpath();
        return path;

    }

    //________________________________________________________________________________________________________
    bool Helper::compositingActive() const
    {

        #if BREEZE_HAVE_X11
        if( isX11() )
        {
            // direct call to X
            xcb_get_selection_owner_cookie_t cookie( xcb_get_selection_owner( connection(), _compositingManagerAtom ) );
            ScopedPointer<xcb_get_selection_owner_reply_t> reply( xcb_get_selection_owner_reply( connection(), cookie, nullptr ) );
            return reply && reply->owner;

        }
        #endif

        // use KWindowSystem
        return KWindowSystem::compositingActive();

    }

    //____________________________________________________________________
    bool Helper::hasAlphaChannel( const QWidget* widget ) const
    { return compositingActive() && widget && widget->testAttribute( Qt::WA_TranslucentBackground ); }

    //______________________________________________________________________________________
    QPixmap Helper::highDpiPixmap( int width, int height ) const
    {
        #if QT_VERSION >= 0x050300
        const qreal dpiRatio( qApp->devicePixelRatio() );
        QPixmap pixmap( width*dpiRatio, height*dpiRatio );
        pixmap.setDevicePixelRatio( dpiRatio );
        return pixmap;
        #else
        return QPixmap( width, height );
        #endif
    }

    //______________________________________________________________________________________
    qreal Helper::devicePixelRatio( const QPixmap& pixmap ) const
    {
        #if QT_VERSION >= 0x050300
        return pixmap.devicePixelRatio();
        #else
        Q_UNUSED(pixmap);
        return 1;
        #endif
    }

    #if BREEZE_HAVE_X11

    //____________________________________________________________________
    xcb_connection_t* Helper::connection()
    {

        #if QT_VERSION >= 0x050000
        return QX11Info::connection();
        #else
        static xcb_connection_t* connection = nullptr;
        if( !connection )
        {
            Display* display = QX11Info::display();
            if( display ) connection = XGetXCBConnection( display );
        }
        return connection;
        #endif
    }

    //____________________________________________________________________
    xcb_atom_t Helper::createAtom( const QString& name ) const
    {
        if( isX11() )
        {

            xcb_connection_t* connection( Helper::connection() );
            xcb_intern_atom_cookie_t cookie( xcb_intern_atom( connection, false, name.size(), qPrintable( name ) ) );
            ScopedPointer<xcb_intern_atom_reply_t> reply( xcb_intern_atom_reply( connection, cookie, nullptr) );
            return reply ? reply->atom:0;

        } else return 0;

    }

    #endif

    //____________________________________________________________________
    void Helper::init()
    {
        #if BREEZE_HAVE_X11

        if( isX11() )
        {
            // create compositing screen
            const QString atomName( QStringLiteral( "_NET_WM_CM_S%1" ).arg( QX11Info::appScreen() ) );
            _compositingManagerAtom = createAtom( atomName );
        }

        #endif

    }

}
