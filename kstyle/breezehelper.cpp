/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#endif

#include <algorithm>
#include <QDialog>


namespace Breeze
{

    //* contrast for arrow and treeline rendering
    static const qreal arrowShade = 0.15;

    static const qreal highlightBackgroundAlpha = 0.33;

    static const auto radioCheckSunkenDarkeningFactor = 110;

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

        const QList<QPalette::ColorRole> roles = { QPalette::Window, QPalette::Highlight, QPalette::WindowText, QPalette::ButtonText, QPalette::Text, QPalette::Button };
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
    void Helper::renderFrameWithSides(
        QPainter* painter, const QRect& rect,
        const QColor& color, Qt::Edges edges,
        const QColor& outline ) const
    {

        painter->save();

        painter->setRenderHint( QPainter::Antialiasing );

        QRectF frameRect( rect );

        // set brush
        painter->setBrush( color );
        painter->setPen( Qt::NoPen );

        // render
        painter->drawRect( frameRect );

        // set brush again
        painter->setBrush( Qt::NoBrush );
        painter->setPen( outline );

        // manually apply the effects of StrokedRect here but only to the edges with a frame
        if ( edges & Qt::LeftEdge ) {
            frameRect.adjust(0.5, 0.0, 0.0, 0.0);
        }
        if ( edges & Qt::RightEdge ) {
            frameRect.adjust(0.0, 0, -0.5, 0.0);
        }
        if ( edges & Qt::TopEdge ) {
            frameRect.adjust(0.0, 0.5, 0.0, 0.0);
        }
        if ( edges & Qt::BottomEdge ) {
            frameRect.adjust(0.0, 0.0, 0.0, -0.5);
        }

        // draw lines
        if ( edges & Qt::LeftEdge ) {
            painter->drawLine( QLineF(frameRect.topLeft(), frameRect.bottomLeft()) );
        }
        if ( edges & Qt::RightEdge ) {
            painter->drawLine( QLineF(frameRect.topRight(), frameRect.bottomRight()) );
        }
        if ( edges & Qt::TopEdge ) {
            painter->drawLine( QLineF(frameRect.topLeft(), frameRect.topRight()) );
        }
        if ( edges & Qt::BottomEdge ) {
            painter->drawLine( QLineF(frameRect.bottomLeft(), frameRect.bottomRight()) );
        }

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
    void Helper::renderButtonFrame(QPainter* painter, const QRect& rect, const QPalette& palette, const QHash<QByteArray, bool>& stateProperties, qreal bgAnimation, qreal penAnimation) const
    {
        bool enabled = stateProperties.value("enabled", true);
        bool visualFocus = stateProperties.value("visualFocus");
        bool hovered = stateProperties.value("hovered");
        bool down = stateProperties.value("down");
        bool checked = stateProperties.value("checked");
        bool flat = stateProperties.value("flat");
        bool defaultButton = stateProperties.value("defaultButton");
        bool hasNeutralHighlight = stateProperties.value("hasNeutralHighlight");

        // don't render background if flat and not hovered, down, checked, or given visual focus
        if (flat && !(hovered || down || checked || visualFocus)
            && bgAnimation == AnimationData::OpacityInvalid
            && penAnimation == AnimationData::OpacityInvalid
        ) {
            return;
        }

        QRectF shadowedRect = this->shadowedRect(rect);
        QRectF frameRect = strokedRect(shadowedRect);
        qreal radius = frameRadius( PenWidth::Frame );
        // setting color group to work around KColorScheme feature
        const QColor &highlightColor = palette.color(!enabled ? QPalette::Disabled : QPalette::Active, QPalette::Highlight);
        QBrush bgBrush;
        QBrush penBrush;

        // Colors
        if (flat) {
            if (down && enabled) {
                bgBrush = alphaColor(highlightColor, highlightBackgroundAlpha);
            } else if (checked) {
                bgBrush = hasNeutralHighlight ? alphaColor(neutralText(palette), highlightBackgroundAlpha)
                    : alphaColor(palette.buttonText().color(), 0.125);
                penBrush = hasNeutralHighlight ? neutralText(palette)
                    : KColorUtils::mix(palette.button().color(), palette.buttonText().color(), 0.3);
            } else if (defaultButton) {
                bgBrush = alphaColor(highlightColor, 0.125);
                penBrush = KColorUtils::mix(
                    highlightColor,
                    KColorUtils::mix(palette.button().color(), palette.buttonText().color(), 0.333),
                    0.5
                );
            } else {
                bgBrush = alphaColor(highlightColor, 0);
                penBrush = hasNeutralHighlight ? neutralText(palette) : bgBrush;
            }
        } else {
            if (down && enabled) {
                bgBrush = KColorUtils::mix(palette.button().color(), highlightColor, 0.333);
            } else if (checked) {
                bgBrush = hasNeutralHighlight ? KColorUtils::mix(palette.button().color(), neutralText(palette), 0.333)
                    : KColorUtils::mix(palette.button().color(), palette.buttonText().color(), 0.125);
                penBrush = hasNeutralHighlight ? neutralText(palette)
                    : KColorUtils::mix(palette.button().color(), palette.buttonText().color(), 0.3);
            } else if (defaultButton) {
                bgBrush = KColorUtils::mix(palette.button().color(), highlightColor, 0.2);
                penBrush = KColorUtils::mix(
                    highlightColor,
                    KColorUtils::mix(palette.button().color(), palette.buttonText().color(), 0.333),
                    0.5
                );
            } else {
                bgBrush = palette.button().color();
                penBrush = hasNeutralHighlight ? neutralText(palette)
                    : KColorUtils::mix(palette.button().color(), palette.buttonText().color(), 0.3);
            }
        }

        if ((hovered || visualFocus || down) && enabled) {
            penBrush = highlightColor;
        }

        // Animations
        if (bgAnimation != AnimationData::OpacityInvalid && enabled) {
            QColor color1 = bgBrush.color();
            QColor color2 = flat ? alphaColor(highlightColor, highlightBackgroundAlpha)
                : KColorUtils::mix(palette.button().color(), highlightColor, 0.333);
            bgBrush = KColorUtils::mix(color1, color2, bgAnimation);
        }
        if (penAnimation != AnimationData::OpacityInvalid && enabled) {
            QColor color1 = penBrush.color();
            QColor color2 = highlightColor;
            penBrush = KColorUtils::mix(color1, color2, penAnimation);
        }

        // Gradient
        if (!(flat || down || hovered || checked) && enabled) {
            QLinearGradient bgGradient(frameRect.topLeft(), frameRect.bottomLeft());
            bgGradient.setColorAt(0, KColorUtils::mix(bgBrush.color(), Qt::white, 0.03125));
            bgGradient.setColorAt(0.5, bgBrush.color());
            bgGradient.setColorAt(1, KColorUtils::mix(bgBrush.color(), Qt::black, 0.03125));
            QLinearGradient penGradient(frameRect.topLeft(), frameRect.bottomLeft());
            penGradient.setColorAt(0, KColorUtils::mix(penBrush.color(), Qt::white, 0.03125));
            penGradient.setColorAt(1, KColorUtils::mix(penBrush.color(), Qt::black, 0.0625));
            bgBrush = bgGradient;
            penBrush = penGradient;
        }

        // Shadow
        if (!(flat || down || checked) && enabled) {
            renderRoundedRectShadow(painter, shadowedRect, shadowColor(palette));
        }

        // Render button
        painter->setRenderHint( QPainter::Antialiasing, true );
        painter->setBrush(bgBrush);
        painter->setPen(QPen(penBrush, PenWidth::Frame));
        painter->drawRoundedRect(frameRect, radius, radius);
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
        CheckBoxState state, bool neutalHighlight,
        bool sunken, qreal animation ) const
    {
        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );
        frameRect = strokedRect(frameRect);

        auto transparent = neutalHighlight ? neutralText(palette) : palette.highlight().color();
        transparent.setAlphaF(highlightBackgroundAlpha);

        QBrush penBrush;
        if (neutalHighlight) {
            penBrush = neutralText(palette);
        } else if (state == CheckOn || state == CheckPartial) {
            penBrush = palette.highlight().color();
        } else {
            penBrush = transparentize(palette.text().color(), highlightBackgroundAlpha);
        }
        painter->setPen(QPen(penBrush, PenWidth::Frame));

        const auto radius = Metrics::CheckBox_Radius;

        switch (state) {
        case CheckOff:
            painter->setBrush( palette.base().color().darker(sunken ? radioCheckSunkenDarkeningFactor : 100) );
            painter->drawRoundedRect( frameRect, radius, radius );
            break;

        case CheckPartial:
        case CheckOn:
            painter->setBrush( transparent );
            painter->drawRoundedRect( frameRect, radius, radius );
            break;

        case CheckAnimated:
            painter->setBrush( palette.base().color().darker(sunken ? radioCheckSunkenDarkeningFactor : 100) );
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
        bool neutalHighlight, bool sunken,
        qreal animation, qreal hoverAnimation ) const
    {
        Q_UNUSED(sunken)

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
    void Helper::renderRadioButtonBackground( QPainter* painter, const QRect& rect, const QPalette& palette, RadioButtonState state, bool neutalHighlight, bool sunken, qreal animation ) const
    {

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );

        // copy rect
        QRectF frameRect( rect );
        frameRect.adjust( 2, 2, -2, -2 );
        frameRect.adjust( 0.5, 0.5, -0.5, -0.5 );

        auto transparent = neutalHighlight ? neutralText(palette) : palette.highlight().color();
        transparent.setAlphaF(highlightBackgroundAlpha);

        QBrush penBrush;
        if (neutalHighlight) {
            penBrush = neutralText(palette);
        } else if (state == RadioOn) {
            penBrush = palette.highlight().color();
        } else {
            penBrush = transparentize(palette.text().color(), highlightBackgroundAlpha);
        }
        painter->setPen(QPen(penBrush, PenWidth::Frame));

        switch (state) {
        case RadioOff:
            painter->setBrush( palette.base().color().darker(sunken ? radioCheckSunkenDarkeningFactor : 100) );
            painter->drawEllipse( frameRect );
            break;
        case RadioOn:
            painter->setBrush( transparent );
            painter->drawEllipse( frameRect );
            break;
        case RadioAnimated:
            painter->setBrush( palette.base().color().darker(sunken ? radioCheckSunkenDarkeningFactor : 100) );
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
        RadioButtonState state, bool neutralHighlight, bool sunken, qreal animation, qreal animationHover ) const
    {
        Q_UNUSED(sunken)

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
        markerRect = frameRect.adjusted( 6, 6, -6, -6 );
        
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
            painter->setPen( QPen(color, PenWidth::Frame) );
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

            painter->setPen( QPen(outline, PenWidth::Frame) );
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
            painter->setPen( QPen(fg, PenWidth::Frame) );
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
    void Helper::renderTabBarTab(QPainter *painter, const QRect &rect, const QPalette& palette,
                                 const QHash<QByteArray, bool>& stateProperties, Corners corners, qreal animation) const
    {
        bool enabled = stateProperties.value("enabled", true);
        bool visualFocus = stateProperties.value("visualFocus");
        bool hovered = stateProperties.value("hovered");
        bool down = stateProperties.value("down");
        bool selected = stateProperties.value("selected");
        bool documentMode = stateProperties.value("documentMode");
        bool north = stateProperties.value("north");
        bool south = stateProperties.value("south");
        bool west = stateProperties.value("west");
        bool east = stateProperties.value("east");
        bool animated = animation != AnimationData::OpacityInvalid;
        bool isQtQuickControl = stateProperties.value("isQtQuickControl");
        bool hasAlteredBackground = stateProperties.value("hasAlteredBackground");

        // setup painter
        painter->setRenderHint( QPainter::Antialiasing, true );
        QRectF frameRect = rect;
        QColor bgBrush;

        if (selected) {
            if (north) {
                // overlap bottom border
                frameRect.adjust(0, 0, 0, 1);
            } else if (south) {
                // overlap top border
                frameRect.adjust(0, -1, 0, 0);
            } else if (west) {
                // overlap right border
                frameRect.adjust(0, 0, 1, 0);
            } else if (east) {
                // overlap left border
                frameRect.adjust(-1, 0, 0, 0);
            }
            if (documentMode && !isQtQuickControl && !hasAlteredBackground) {
                bgBrush = palette.color(QPalette::Window);
            } else {
                bgBrush = frameBackgroundColor(palette);
            }
            QColor penBrush = KColorUtils::mix(bgBrush, palette.color(QPalette::WindowText), 0.25);
            painter->setBrush(bgBrush);
            painter->setPen(QPen(penBrush, PenWidth::Frame));
            QRectF highlightRect = frameRect;
            if (north || south) {
                highlightRect.setHeight(Metrics::Frame_FrameRadius);
            } else if (west || east) {
                highlightRect.setWidth(Metrics::Frame_FrameRadius);
            }
            if (south) {
                highlightRect.moveBottom(frameRect.bottom());
            } else if (east) {
                highlightRect.moveRight(frameRect.right());
            }
            QPainterPath path = roundedPath(strokedRect(frameRect), corners, frameRadius(PenWidth::Frame));
            painter->drawPath(path);
            QPainterPath highlightPath = roundedPath(highlightRect, corners, Metrics::Frame_FrameRadius);
            painter->setBrush(palette.color(QPalette::Highlight));
            painter->setPen(Qt::NoPen);
            painter->drawPath(highlightPath);
        } else {
            if (north) {
                // don't overlap bottom border
                frameRect.adjust(0, 0, 0, -1);
            } else if (south) {
                // don't overlap top border
                frameRect.adjust(0, 1, 0, 0);
            } else if (west) {
                // don't overlap right border
                frameRect.adjust(0, 0, -1, 0);
            } else if (east) {
                // don't overlap left border
                frameRect.adjust(1, 0, 0, 0);
            }
            const auto windowColor = palette.color(QPalette::Window);
            bgBrush = windowColor.darker(windowColor.lightnessF() > 0.5 ? 120 : 160);
            const auto hover = alphaColor(hoverColor(palette), 0.2);
            if (animated) {
                bgBrush = KColorUtils::mix(bgBrush, hover, animation);
            } else if (enabled && hovered && !selected) {
                bgBrush = hover;
            }
            painter->setBrush(bgBrush);
            painter->setPen(Qt::NoPen);
            QPainterPath path = roundedPath(frameRect, corners, Metrics::Frame_FrameRadius);
            painter->drawPath(path);
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
    void Helper::renderDecorationButton( QPainter* painter, const QRect& rect, const QColor& foregroundColor, ButtonType buttonType, bool inverted, bool paintBackground, const QColor& backgroundColor, const QColor& outlineColor ) const
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
            // render circle or square background
            if( outlineColor.isValid() ) painter->setPen( outlineColor );
            else painter->setPen( Qt::NoPen );
            
            if( inverted ) painter->setBrush( foregroundColor );
            else painter->setBrush( backgroundColor );
            
            if( decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeFullHeightRectangle
                || decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeSmallSquare
                || decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare
                || decorationConfig()->buttonShape() ==  InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle
            )
                painter->drawRoundedRect( QRectF( 2, 2, 14, 14 ), 20, 20, Qt::RelativeSize );
            else {
                if( outlineColor.isValid() ) painter->drawEllipse( QRectF( 1, 1, 16, 16 ) ); // have to shrink outlined circle otherwise it gets clipped
                else painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
            }
            
            
            if( inverted ){
                // take out the inner part
                painter->setCompositionMode( QPainter::CompositionMode_DestinationOut );
                painter->setBrush( Qt::NoBrush );
                pen.setColor( Qt::black );
            } else {
                painter->setBrush( Qt::NoBrush );
                if (buttonType != ButtonClose && !decorationConfig()->translucentButtonBackgrounds() ){ //don't want to ruin the nice white colour in the close button
                    QColor higherContrastForegroundColor = ColorTools::getHigherContrastForegroundColor( foregroundColor, backgroundColor, 2.7);
                    pen.setColor( higherContrastForegroundColor );
                } else pen.setColor( foregroundColor );
            }

        } else {

            painter->setBrush( Qt::NoBrush );
            pen.setColor( foregroundColor );

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

        painter->setRenderHint( QPainter::Antialiasing, true );

        qreal adjustment = 0.5 * PenWidth::Shadow; // Translate for the pen
        QRectF shadowRect = rect.adjusted( adjustment, adjustment, -adjustment, adjustment );

        painter->setPen( QPen(color, PenWidth::Shadow) );
        painter->setBrush( Qt::NoBrush );
        painter->drawRoundedRect( shadowRect, radius, radius );
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
    QRectF Helper::strokedRect( const QRectF &rect, const qreal penWidth ) const
    {
        /* With a pen stroke width of 1, the rectangle should have each of its
         * sides moved inwards by half a pixel. This allows the stroke to be
         * pixel perfect instead of blurry from sitting between pixels and
         * prevents the rectangle with a stroke from becoming larger than the
         * original size of the rectangle.
         */
        qreal adjustment = 0.5 * penWidth;
        return rect.adjusted( adjustment, adjustment, -adjustment, -adjustment );
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
        /*if (borderSize != "None" && borderSize != "NoSides") {
            return false;
        }*/ //edited out for classik as we can set the borders to titlebar colour to avoid any such visual glitches
        return true;
    }
}
