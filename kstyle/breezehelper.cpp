/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezehelper.h"

#include "breeze.h"
#include "breezestyleconfigdata.h"
#include "renderdecorationbuttonicon.h"
#include "colortools.h"

#include <KColorUtils>
#include <KIconLoader>
#include <KWindowSystem>

#include <QApplication>
#include <QDBusConnection>
#include <QFileInfo>
#include <QPainter>
#include <QMainWindow>
#include <QMenuBar>
#include <QMdiArea>
#include <QDockWidget>
#include <QWindow>

#if BREEZE_HAVE_QTX11EXTRAS
#include <QX11Info>
#endif

#include <algorithm>
#include <QDialog>


namespace Breeze
{

    //* contrast for arrow and treeline rendering
    static const qreal arrowShade = 0.15;

    //____________________________________________________________________
    Helper::Helper( KSharedConfig::Ptr config, QObject *parent ) :
        QObject ( parent ),
        _config( std::move( config ) ),
        _kwinConfig( KSharedConfig::openConfig("kwinrc") ),
        _decorationConfig( new InternalSettings() )
    {
        if (qApp) {
            connect(qApp, &QApplication::paletteChanged, this, [=]() {
                if (qApp->property("KDE_COLOR_SCHEME_PATH").isValid()) {
                    const auto path = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
                    KConfig config(path, KConfig::SimpleConfig);
                    KConfigGroup group( config.group("WM") );
                    const QPalette palette( QApplication::palette() );
                    _activeTitleBarColor = group.readEntry( "activeBackground", palette.color( QPalette::Active, QPalette::Highlight ) );
                    _activeTitleBarTextColor = group.readEntry( "activeForeground", palette.color( QPalette::Active, QPalette::HighlightedText ) );
                    _inactiveTitleBarColor = group.readEntry( "inactiveBackground", palette.color( QPalette::Disabled, QPalette::Highlight ) );
                    _inactiveTitleBarTextColor = group.readEntry( "inactiveForeground", palette.color( QPalette::Disabled, QPalette::HighlightedText ) );
                }
            });
        }
    }

    //____________________________________________________________________
    KSharedConfig::Ptr Helper::config() const
    { return _config; }


    //____________________________________________________________________
    QSharedPointer<InternalSettings> Helper::decorationConfig() const
    { return _decorationConfig; }

    //____________________________________________________________________
    void Helper::loadConfig()
    {
        _viewFocusBrush = KStatefulBrush( KColorScheme::View, KColorScheme::FocusColor );
        _viewHoverBrush = KStatefulBrush( KColorScheme::View, KColorScheme::HoverColor );
        _buttonFocusBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::FocusColor );
        _buttonHoverBrush = KStatefulBrush( KColorScheme::Button, KColorScheme::HoverColor );
        _viewNegativeTextBrush = KStatefulBrush( KColorScheme::View, KColorScheme::NegativeText );
        _viewNeutralTextBrush = KStatefulBrush( KColorScheme::View, KColorScheme::NeutralText );

        const QPalette palette( QApplication::palette() );
        _config->reparseConfiguration();
        _kwinConfig->reparseConfiguration();
        _cachedAutoValid = false;
        _decorationConfig->load();

        KConfig config(qApp->property("KDE_COLOR_SCHEME_PATH").toString(), KConfig::SimpleConfig);
        KConfigGroup appGroup( config.group("WM") );
        KConfigGroup globalGroup( _config->group("WM") );
        _activeTitleBarColor = appGroup.readEntry( "activeBackground", globalGroup.readEntry( "activeBackground", palette.color( QPalette::Active, QPalette::Highlight ) ) );
        _activeTitleBarTextColor = appGroup.readEntry( "activeForeground", globalGroup.readEntry( "activeForeground", palette.color( QPalette::Active, QPalette::HighlightedText ) ) );
        _inactiveTitleBarColor = appGroup.readEntry( "inactiveBackground", globalGroup.readEntry( "inactiveBackground", palette.color( QPalette::Disabled, QPalette::Highlight ) ) );
        _inactiveTitleBarTextColor = appGroup.readEntry( "inactiveForeground", globalGroup.readEntry( "inactiveForeground", palette.color( QPalette::Disabled, QPalette::HighlightedText ) ) );
    }

    QColor transparentize(const QColor& color, qreal amount)
    {
        auto clone = color;
        clone.setAlphaF(amount);
        return clone;
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
    { return KColorUtils::mix( buttonFocusColor( palette ), palette.color( QPalette::ButtonText ), 0.15 ); }

    //____________________________________________________________________
    QColor Helper::buttonHoverOutlineColor( const QPalette& palette ) const
    { return KColorUtils::mix( buttonHoverColor( palette ), palette.color( QPalette::ButtonText ), 0.15 ); }

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

                const QColor hover( buttonHoverColor( palette ) );
                outline = KColorUtils::mix( outline, hover, opacity );

            }

        } else if( mouseOver ) {

            if( hasFocus ) outline = buttonHoverOutlineColor( palette );
            else outline = buttonHoverColor( palette );

        } else if( mode == AnimationFocus ) {

            const QColor focus( buttonFocusOutlineColor( palette ) );
            outline = KColorUtils::mix( outline, focus, opacity );

        } else if( hasFocus ) {

            outline = buttonFocusOutlineColor( palette );

        }

        return outline;

    }

    //____________________________________________________________________
    QPair<QColor,QColor> Helper::buttonBackgroundColor( const QPalette& palette, bool mouseOver, bool hasFocus, bool sunken, qreal opacity, AnimationMode mode ) const
    {

        QColor background( sunken ?
            KColorUtils::mix( palette.color( QPalette::Button ), Qt::black, KColorUtils::luma( palette.color(QPalette::Button) ) < 0.5 ? 0.3 : 0.10 ):
            palette.color( QPalette::Button ) );
        auto base = palette.color(QPalette::Button);

        return {background, base};

    }

    //____________________________________________________________________
    QColor Helper::toolButtonColor( const QPalette& palette, bool mouseOver, bool hasFocus, bool sunken, qreal opacity, AnimationMode mode ) const
    {

        QColor outline;
        const QColor hoverColor( buttonHoverColor( palette ) );
        const QColor focusColor( buttonFocusColor( palette ) );
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
        painter->drawRect( strokedRect( rect ) );
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

            QRectF copy( strokedRect( rect ) );

            const qreal radius( frameRadius( PenWidth::Frame ) );
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
        qreal radius( frameRadius( PenWidth::NoPen ) );

        // set pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect = strokedRect( frameRect );
            radius = frameRadiusForNewPenWidth( radius, PenWidth::Frame );

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
        QRectF frameRect( strokedRect( rect ) );

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
                const qreal radius( frameRadius( PenWidth::Frame ) );
                painter->drawRoundedRect( frameRect, radius, radius );
                break;
            }

        }

    }

    //______________________________________________________________________________
    void Helper::renderMenuFrame(
        QPainter* painter, const QRect& rect,
        const QColor& color, const QColor& outline, bool roundCorners, bool isTopMenu ) const
    {


        painter->save();

        // set brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        // We simulate being able to independently adjust corner radii by
        // setting a clip region and then extending the rectangle beyond it.
        if ( isTopMenu ) {
            painter->setClipRect( rect );
        }

        if( roundCorners )
        {

            painter->setRenderHint( QPainter::Antialiasing );
            QRectF frameRect( rect );
            qreal radius( frameRadius( PenWidth::NoPen ) );

            if( isTopMenu ) frameRect.adjust(0, -radius, 0, 0);

            // set pen
            if( outline.isValid() )
            {

                painter->setPen( outline );
                frameRect = strokedRect( frameRect );
                radius = frameRadiusForNewPenWidth( radius, PenWidth::Frame );

            } else painter->setPen( Qt::NoPen );

            // render
            painter->drawRoundedRect( frameRect, radius, radius );

        } else {

            painter->setRenderHint( QPainter::Antialiasing, false );
            QRect frameRect( rect );
            if( isTopMenu ) frameRect.adjust(0, 1, 0, 0);

            if( outline.isValid() )
            {

                painter->setPen( outline );
                frameRect.adjust( 0, 0, -1, -1 );

            } else painter->setPen( Qt::NoPen );

            painter->drawRect( frameRect );

        }

        painter->restore();

    }

    //______________________________________________________________________________
    void Helper::renderButtonFrame(
        QPainter* painter, const QRect& rect,
        const QPair<QColor,QColor>& color, const QColor& outline, const QColor& shadow,
        bool hasFocus, bool sunken ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 1, 1, -1, -1 );
        qreal radius( frameRadius( PenWidth::NoPen ) );

        // shadow
        if( sunken ) {

            frameRect.translate( 1, 1 );

        } else {

            renderRoundedRectShadow( painter, frameRect, shadow, radius );

        }

        if( outline.isValid() )
        {

            QLinearGradient gradient( frameRect.topLeft(), frameRect.bottomLeft() );
            gradient.setColorAt( 0, outline.lighter( hasFocus ? 103:101 ) );
            gradient.setColorAt( 1, outline.darker( hasFocus ? 110:103 ) );
            painter->setPen( QPen( QBrush( gradient ), 1.0 ) );

            frameRect = strokedRect( frameRect );
            radius = frameRadiusForNewPenWidth( radius, PenWidth::Frame );

        } else painter->setPen( Qt::NoPen );

        // content
        if( color.first.isValid() && color.second.isValid() )
        {

            QLinearGradient gradient( frameRect.topLeft(), frameRect.bottomLeft() );
            gradient.setColorAt( 0, color.first );
            gradient.setColorAt( 1, color.second );
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

        const QRectF baseRect( rect.adjusted( 1, 1, -1, -1 ) );
        const qreal radius( frameRadius( PenWidth::Frame ) );

        if( sunken )
        {

            painter->setPen( color );
            painter->setBrush( alphaColor(color, 0.35) );

            painter->drawRoundedRect( strokedRect ( baseRect ), radius, radius );

        } else {

            painter->setPen( color );
            painter->setBrush( Qt::NoBrush );
            const QRectF outlineRect( strokedRect( baseRect ) );
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
        const qreal radius( frameRadius( PenWidth::Frame ) );
        const QSizeF cornerSize( 2*radius, 2*radius );

        // if rect - tabwidth is even, need to increase tabWidth by 1 unit
        // for anti aliasing
        if( !((rect.width() - tabWidth)%2) ) ++tabWidth;

        // adjust rect for antialiasing
        QRectF baseRect( strokedRect( rect ) );

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
        qreal radius( frameRadius( PenWidth::NoPen ) );

        // set pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect = strokedRect( frameRect );
            radius = frameRadiusForNewPenWidth( radius, PenWidth::Frame );

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
        const QPalette& palette,
        CheckBoxState state, bool neutalHighlight, qreal animation ) const
    {
        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );
        frameRect = strokedRect(frameRect);

        auto transparent = neutalHighlight ? neutralText(palette) : palette.highlight().color();
        transparent.setAlphaF(0.50);

        painter->setPen( transparentize( palette.text().color(), 0.5 ) );
        if (state == CheckOn || state == CheckPartial) {
            painter->setPen( neutalHighlight ? neutralText(palette) : palette.highlight().color() );
        }

        const auto radius = Metrics::CheckBox_Radius;

        switch (state) {
        case CheckOff:
            painter->setBrush( palette.base() );
            painter->drawRoundedRect( frameRect, radius, radius );
            break;

        case CheckPartial:
        case CheckOn:
            painter->setBrush( transparent );
            painter->drawRoundedRect( frameRect, radius, radius );
            break;

        case CheckAnimated:
            painter->setBrush( palette.base() );
            painter->drawRoundedRect( frameRect, radius, radius );
            painter->setBrush( transparent );
            painter->setOpacity( animation );
            painter->drawRoundedRect( frameRect, radius, radius );
            break;
        }

    }

    //______________________________________________________________________________
    void Helper::renderCheckBox(
        QPainter* painter, const QRect& rect,
        const QPalette& palette, bool mouseOver,
        CheckBoxState state, CheckBoxState target,
        bool neutalHighlight,
        qreal animation, qreal hoverAnimation ) const
    {
        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect and radius
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );

        if ( mouseOver ) {
            painter->save();

            if (hoverAnimation != AnimationData::OpacityInvalid) {
                painter->setOpacity(hoverAnimation);
            }

            painter->setPen( QPen( neutalHighlight ? neutralText(palette).lighter() : focusColor(palette), PenWidth::Frame ) );
            painter->setBrush( Qt::NoBrush );

            painter->drawRoundedRect( frameRect.adjusted(0.5, 0.5, -0.5, -0.5), Metrics::CheckBox_Radius, Metrics::CheckBox_Radius );

            painter->restore();
        }

        // check
        auto leftPoint = frameRect.center();
        leftPoint.setX(frameRect.left()+4);

        auto bottomPoint = frameRect.center();
        bottomPoint.setX(bottomPoint.x() - 1);
        bottomPoint.setY(frameRect.bottom() - 5);

        auto rightPoint = frameRect.center();
        rightPoint.setX(rightPoint.x() + 4.5);
        rightPoint.setY(frameRect.top() + 5.5);

        QPainterPath path;
        path.moveTo(leftPoint);
        path.lineTo(bottomPoint);
        path.lineTo(rightPoint);

        // dots
        auto centerDot = QRectF(frameRect.center(), QSize(2, 2));
        centerDot.adjust(-1, -1, -1, -1);
        auto leftDot = centerDot.adjusted(-4, 0, -4, 0);
        auto rightDot = centerDot.adjusted(4, 0, 4, 0);

        painter->setPen(Qt::transparent);
        painter->setBrush(Qt::transparent);

        auto checkPen = QPen( palette.text(), PenWidth::Frame * 2 );
        checkPen.setJoinStyle(Qt::MiterJoin);

        switch (state) {
        case CheckOff:
            break;
        case CheckOn:
            painter->setPen( checkPen );
            painter->drawPath( path );
            break;
        case CheckPartial:
            painter->setBrush( palette.text() );
            painter->drawRect(leftDot);
            painter->drawRect(centerDot);
            painter->drawRect(rightDot);
            break;
        case CheckAnimated:
            checkPen.setDashPattern({ path.length() * animation, path.length() });

            switch (target) {
            case CheckOff:
                break;
            case CheckOn:
                painter->setPen( checkPen );
                painter->drawPath( path );
                break;
            case CheckPartial:
                if (animation >= 3/3) painter->drawRect(rightDot);
                if (animation >= 2/3) painter->drawRect(centerDot);
                if (animation >= 1/3) painter->drawRect(leftDot);
                break;
            case CheckAnimated:
                break;
            }
            break;
        }
    }

    //______________________________________________________________________________
    void Helper::renderRadioButtonBackground( QPainter* painter, const QRect& rect, const QPalette& palette, RadioButtonState state, bool neutalHighlight, qreal animation ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );
        frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );

        auto transparent = neutalHighlight ? neutralText(palette) : palette.highlight().color();
        transparent.setAlphaF(0.50);

        painter->setPen( transparentize( palette.text().color(), 0.5 ) );
        if (state == RadioOn) {
            painter->setPen( neutalHighlight ? neutralText(palette) : palette.highlight().color() );
        }

        switch (state) {
        case RadioOff:
            painter->setBrush( palette.base() );
            painter->drawEllipse( frameRect );
            break;
        case RadioOn:
            painter->setBrush( transparent );
            painter->drawEllipse( frameRect );
            break;
        case RadioAnimated:
            painter->setBrush( palette.base() );
            painter->drawEllipse( frameRect );
            painter->setBrush( transparent );
            painter->setOpacity( animation );
            painter->drawEllipse( frameRect );
            break;
        }
    }

    //______________________________________________________________________________
    void Helper::renderRadioButton(
        QPainter* painter, const QRect& rect,
        const QPalette& palette, bool mouseOver,
        RadioButtonState state, bool neutralHighlight, qreal animation, qreal animationHover ) const
    {
        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 1, 1, -1, -1 );

        if ( mouseOver ) {
            painter->save();

            if (animationHover != AnimationData::OpacityInvalid) {
                painter->setOpacity(animationHover);
            }

            painter->setPen( QPen( neutralHighlight ? neutralText(palette).lighter() : focusColor(palette), PenWidth::Frame ) );
            painter->setBrush( Qt::NoBrush );

            const QRectF contentRect( frameRect.adjusted( 1, 1, -1, -1 ).adjusted( 0.5 , 0.5, -0.5, -0.5 ) );
            painter->drawEllipse( contentRect );

            painter->restore();
        }

        painter->setBrush( palette.text() );
        painter->setPen( Qt::NoPen );

        QRectF markerRect;
        markerRect = frameRect.adjusted( 5, 5, -5, -5 );
        
        qreal adjustFactor;

        // mark
        switch (state) {
        case RadioOn:
            painter->drawEllipse( markerRect );

            break;
        case RadioAnimated:
            adjustFactor = markerRect.height() * (1 - animation);
            markerRect.adjust(adjustFactor, adjustFactor, -adjustFactor, -adjustFactor);
            painter->drawEllipse( markerRect );

            break;
        default:
            break;
        }
    }

    //______________________________________________________________________________
    void Helper::renderSliderGroove(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        QRectF baseRect( rect );
        baseRect.adjust(0.5, 0.5, -0.5, -0.5);
        const qreal radius( 0.5*Metrics::Slider_GrooveThickness );

        // content
        if( color.isValid() )
        {
            painter->setPen( color );
            auto bg = color;
            bg.setAlphaF(bg.alphaF() / 2);
            painter->setBrush( bg );
            painter->drawRoundedRect( baseRect, radius, radius );
        }

        
    }

    //______________________________________________________________________________
    void Helper::renderDialGroove(
        QPainter* painter, const QRect& rect,
        const QColor& fg, const QColor& bg,
        qreal first, qreal last ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        const QRectF baseRect( rect );

        // content
        if( fg.isValid() )
        {
            const qreal penWidth( Metrics::Slider_GrooveThickness );
            const QRectF grooveRect( rect.adjusted( penWidth/2, penWidth/2, -penWidth/2, -penWidth/2 ) );

            // setup angles
            const int angleStart( first * 180 * 16 / M_PI );
            const int angleSpan( (last - first ) * 180 * 16 / M_PI );

            const QPen bgPen( fg, penWidth, Qt::SolidLine, Qt::RoundCap );
            const QPen fgPen( KColorUtils::overlayColors( bg, alphaColor( fg, 0.5) ), penWidth - 2, Qt::SolidLine, Qt::RoundCap );
            
            // setup pen
            if( angleSpan != 0 )
            {
                painter->setPen( bgPen );
                painter->setBrush( Qt::NoBrush );
                painter->drawArc( grooveRect, angleStart, angleSpan );
                painter->setPen( fgPen );
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
        if( !sunken )
        {

            renderEllipseShadow( painter, frameRect, shadow );

        }

        // set pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect = strokedRect( frameRect );

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
        const QColor& fg, const QColor& bg ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        QRectF baseRect( rect );
        baseRect.adjust(0.5,0.5,-0.5,-0.5);
        const qreal radius( 0.5*Metrics::ProgressBar_Thickness );

        // content
        if( fg.isValid() )
        {
            painter->setPen( fg );
            painter->setBrush( KColorUtils::overlayColors(bg, alphaColor(fg, 0.5)) );
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
    void Helper::renderScrollBarBorder(
        QPainter* painter, const QRect& rect,
        const QColor& color ) const
    {

        // content
        if( color.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( color );
            painter->drawRect( rect );
        }

    }

    //______________________________________________________________________________
    void Helper::renderTabBarTab(QPainter *painter, const QRect &rect,
                                const QColor &color, const QColor &highlight,
                                const QColor &outline, Corners corners,
                                bool document, bool bottom) const {


        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        QRectF frameRect( rect );
        qreal radius( frameRadius( PenWidth::NoPen ) );

        // pen
        if( outline.isValid() )
        {

            painter->setPen( outline );
            frameRect = strokedRect( frameRect );
            radius = frameRadiusForNewPenWidth( radius, PenWidth::Frame );

        } else painter->setPen( Qt::NoPen );


        // brush
        if( color.isValid() ) painter->setBrush( color );
        else painter->setBrush( Qt::NoBrush );

        // render
        QPainterPath path( roundedPath( frameRect, corners, radius ) );
        painter->drawPath( path );

        if (highlight.isValid() && document) {
            auto rect = frameRect;
            rect.setHeight(2);
            if (bottom) {
                rect.setTop(frameRect.bottom()-2);
                rect.setBottom(frameRect.bottom());
            }

            QPainterPath rectAsPath;
            rectAsPath.addRect(rect);

            painter->setClipPath(path.intersected(rectAsPath));
            painter->setBrush(highlight);
            painter->drawRect(frameRect);
        }

    }

    //______________________________________________________________________________
    void Helper::renderArrow( QPainter* painter, const QRect& rect, const QColor& color, ArrowOrientation orientation ) const
    {
        // define polygon
        QPolygonF arrow;
        switch( orientation )
        {
            /* The inner points of the normal arrows are not on half pixels because
             * they need to have an even width (up/down) or height (left/right).
             * An even width/height makes them easier to align with other UI elements.
             */
            case ArrowUp: arrow = QVector<QPointF>{QPointF( -4.5, 1.5 ), QPointF( 0, -3 ), QPointF( 4.5, 1.5 )}; break;
            case ArrowDown: arrow = QVector<QPointF>{QPointF( -4.5, -1.5 ), QPointF( 0, 3 ), QPointF( 4.5, -1.5 )}; break;
            case ArrowLeft: arrow = QVector<QPointF>{QPointF( 1.5, -4.5 ), QPointF( -3, 0 ), QPointF( 1.5, 4.5 )}; break;
            case ArrowRight: arrow = QVector<QPointF>{QPointF( -1.5, -4.5 ), QPointF( 3, 0 ), QPointF( -1.5, 4.5 )}; break;
            case ArrowDown_Small: arrow = QVector<QPointF>{QPointF( 1.5, 3.5 ), QPointF( 3.5, 5.5 ), QPointF( 5.5, 3.5 )}; break;
            default: break;
        }

        painter->save();
        painter->setRenderHints( QPainter::Antialiasing );
        painter->translate( QRectF( rect ).center() );
        painter->setBrush( Qt::NoBrush );
        QPen pen( color, PenWidth::Symbol );
        pen.setCapStyle(Qt::SquareCap);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen( pen );
        painter->drawPolyline( arrow );
        painter->restore();
   }

    //______________________________________________________________________________
    void Helper::renderDecorationButton( QPainter* painter, const QRect& rect, const QColor& color, ButtonType buttonType, bool inverted, bool paintBackground, const QColor& backgroundColor ) const
    {
        
        painter->save();
        painter->setViewport( rect );
        painter->setWindow( 0, 0, 18, 18 );
        painter->setRenderHints( QPainter::Antialiasing );

        // initialize pen
        QPen pen;
        pen.setCapStyle( Qt::RoundCap );
        pen.setJoinStyle( Qt::RoundJoin );
        
        if( inverted ) paintBackground = true;
        if( paintBackground )
        {
            // render circle or square highlight
            painter->setPen( Qt::NoPen );
            
            if( inverted ) painter->setBrush( color );
            else painter->setBrush( backgroundColor );
            
            if ( decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeFullSizedRectangle
                || decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeSmallSquare
                || decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare
                || decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeFullSizedRoundedRectangle
            )
                painter->drawRoundedRect( QRectF( 2, 2, 14, 14 ), 20, 20, Qt::RelativeSize );
            else painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
            
            
            if (inverted ){
                // take out the inner part
                painter->setCompositionMode( QPainter::CompositionMode_DestinationOut );
                painter->setBrush( Qt::NoBrush );
                pen.setColor( Qt::black );
            } else {
                painter->setBrush( Qt::NoBrush );
                if (buttonType != ButtonClose ){ //don't want to ruin the nice white colour in the close button
                    QColor higherContrastForegroundColor = ColorTools::getHigherContrastForegroundColor( color, backgroundColor, 2.7);
                    pen.setColor( higherContrastForegroundColor );
                } else pen.setColor( color );
            }

        } else {

            painter->setBrush( Qt::NoBrush );
            pen.setColor( color );

        }

        pen.setWidthF( PenWidth::Symbol*qMax(1.0, 18.0/rect.width() ) );
        painter->setPen(pen);
        
        std::unique_ptr<RenderDecorationButtonIcon18By18> iconRenderer;
        iconRenderer = RenderDecorationButtonIcon18By18::factory( decorationConfig(), painter, true );
        
        switch( buttonType )
        {
            case ButtonClose:
            {
                iconRenderer->renderCloseIcon();
                break;
            }

            case ButtonMaximize:
            {
                iconRenderer->renderMaximizeIcon();
                break;
            }

            case ButtonMinimize:
            {
                iconRenderer->renderMinimizeIcon();
                break;
            }

            case ButtonRestore:
            {
                iconRenderer->renderRestoreIcon();
                break;
            }

            default: break;
        }

        painter->restore();

    }
    
    //______________________________________________________________________________
    void Helper::renderRoundedRectShadow( QPainter* painter, const QRectF& rect, const QColor& color, qreal radius ) const
    {
        if( !color.isValid() ) return;
        
        painter->save();
        
        qreal translation = 0.5 * PenWidth::Shadow; // Translate for the pen
        
        /* Clipping prevents shadows from being visible inside checkboxes.
         * Clipping away unneeded parts here also improves performance by 40-60%
         * versus using just an outline of a rectangle.
         * Tested by looking at the paint analyser in GammaRay.
         */
        // Right side
        QRegion clip( rect.right() - std::ceil( radius ), rect.top(), 
                      std::ceil( radius ) + PenWidth::Shadow, rect.height() );
        // Bottom side
        clip = clip.united( QRegion( rect.left(), rect.bottom() - std::ceil( radius ), 
                                     rect.width(), std::ceil( radius ) + PenWidth::Shadow ) );

        painter->setClipRegion( clip );
        painter->setPen( color );
        painter->setBrush( Qt::NoBrush );
        painter->drawRoundedRect( rect.translated( translation, translation ), radius, radius );
        
        painter->restore();
    }
    
    //______________________________________________________________________________
    void Helper::renderEllipseShadow( QPainter* painter, const QRectF& rect, const QColor& color ) const
    {
        if( !color.isValid() ) return;
        
        painter->save();

        // Clipping does not improve performance here

        qreal adjustment = 0.5 * PenWidth::Shadow; // Adjust for the pen

        qreal radius = rect.width() / 2 - adjustment;
        
        /* The right side is offset by +0.5 for the visible part of the shadow.
         * The other sides are offset by +0.5 or -0.5 because of the pen.
         */
        QRectF shadowRect = rect.adjusted( adjustment, adjustment, adjustment, -adjustment );
        
        painter->translate( rect.center() );
        painter->rotate( 45 );
        painter->translate( -rect.center() );
        painter->setPen( color );
        painter->setBrush( Qt::NoBrush );
        painter->drawRoundedRect( shadowRect, radius, radius );
        
        painter->restore();
    }
    
    //______________________________________________________________________________
    bool Helper::isX11()
    {
        static const bool s_isX11 = KWindowSystem::isPlatformX11();
        return s_isX11;
    }

    //______________________________________________________________________________
    bool Helper::isWayland()
    {
        static const bool s_isWayland = KWindowSystem::isPlatformWayland();
        return s_isWayland;
    }

    //______________________________________________________________________________
    QRectF Helper::strokedRect( const QRectF &rect, const int penWidth ) const
    {
        /* With a pen stroke width of 1, the rectangle should have each of its
         * sides moved inwards by half a pixel. This allows the stroke to be
         * pixel perfect instead of blurry from sitting between pixels and
         * prevents the rectangle with a stroke from becoming larger than the
         * original size of the rectangle.
         */
        qreal adjustment = 0.5 * penWidth;
        return QRectF( rect ).adjusted( adjustment, adjustment, -adjustment, -adjustment );
    }
    
    QRectF Helper::strokedRect( const QRect &rect, const int penWidth ) const
    {
        return strokedRect(QRectF(rect), penWidth);
    }

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

        #if BREEZE_HAVE_QTX11EXTRAS
        if( isX11() )
        { return QX11Info::isCompositingManagerRunning( QX11Info::appScreen() ); }
        #endif

        // use KWindowSystem
        return KWindowSystem::compositingActive();

    }

    //____________________________________________________________________
    bool Helper::hasAlphaChannel( const QWidget* widget ) const
    { return compositingActive() && widget && widget->testAttribute( Qt::WA_TranslucentBackground ); }

    //______________________________________________________________________________________
    qreal Helper::devicePixelRatio( const QPixmap& pixmap ) const
    {
        return pixmap.devicePixelRatio();
    }

    QPixmap Helper::coloredIcon(const QIcon& icon,  const QPalette& palette, const QSize &size, QIcon::Mode mode, QIcon::State state)
    {
        const QPalette activePalette = KIconLoader::global()->customPalette();
        const bool changePalette = activePalette != palette;
        if (changePalette) {
            KIconLoader::global()->setCustomPalette(palette);
        }
        const QPixmap pixmap = icon.pixmap(size, mode, state);
        if (changePalette) {
            if (activePalette == QPalette()) {
                KIconLoader::global()->resetPalette();
            } else {
                KIconLoader::global()->setCustomPalette(activePalette);
            }
        }
        return pixmap;
    }

    bool Helper::shouldDrawToolsArea(const QWidget* widget) const {
        if (!widget) {
            return false;
        }
        static bool isAuto = false;
        static QString borderSize;
        if (!_cachedAutoValid) {
            KConfigGroup kdecorationGroup(_kwinConfig->group("org.kde.kdecoration2"));
            isAuto = kdecorationGroup.readEntry("BorderSizeAuto", true);
            borderSize = kdecorationGroup.readEntry("BorderSize", "Normal");
            _cachedAutoValid = true;
        }
        if (isAuto) {
            auto window = widget->window();
            if (qobject_cast<const QDialog*>(widget)) {
                return true;
            }
            if (window) {
                auto handle = window->windowHandle();
                if (handle) {
                    auto toolbar = qobject_cast<const QToolBar*>(widget);
                    if (toolbar) {
                        if (toolbar->isFloating()) {
                            return false;
                        }
                    }
                    return true;
                }
            } else {
                return false;
            }
        }
        if (borderSize != "None" && borderSize != "NoSides") {
            return false;
        }
        return true;
    }
}
