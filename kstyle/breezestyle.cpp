// krazy:excludeall=qclasses

//////////////////////////////////////////////////////////////////////////////
// breezestyle.cpp
// Breeze widget style for KDE Frameworks
// -------------------
//
// Copyright (C) 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License version 2 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////

#include "breezestyle.h"
#include "breezestyle.moc"

#include "breezeanimations.h"
#include "breezehelper.h"
#include "breezemetrics.h"
#include "breezemnemonics.h"
#include "breezestyleconfigdata.h"
#include "breezewindowmanager.h"

#include <KColorUtils>

#include <QApplication>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QDBusConnection>
#include <QDockWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QTextEdit>
#include <QToolBox>
#include <QToolButton>

Q_LOGGING_CATEGORY(BREEZE, "breeze")

namespace Breeze
{

    //_________________________________________________
    QStyle* Breeze::StylePlugin::create( const QString &key )
    {
        if( key.toLower() == QStringLiteral( "breeze" ) ) return new Style();
        else return nullptr;
    }

    //_________________________________________________
    Breeze::StylePlugin::~StylePlugin()
    {
        // Delete style when using ::exit() otherwise it'll outlive the unloaded plugin and we'll get a crash
        if (qApp) delete qApp->style();
    }

    //______________________________________________________________
    Style::Style( void ):
        _addLineButtons( SingleButton ),
        _subLineButtons( SingleButton ),
        _helper( new Helper( StyleConfigData::self()->sharedConfig() ) ),
        _animations( new Animations( this ) ),
        _mnemonics( new Mnemonics( this ) ),
        _windowManager( new WindowManager( this ) ),
        SH_ArgbDndWindow( newStyleHint( QStringLiteral( "SH_ArgbDndWindow" ) ) ),
        CE_CapacityBar( newControlElement( QStringLiteral( "CE_CapacityBar" ) ) )

    {

        // use DBus connection to update on breeze configuration change
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.connect( QString(),
            QStringLiteral( "/BreezeStyle" ),
            QStringLiteral( "org.kde.Breeze.Style" ),
            QStringLiteral( "reparseConfiguration" ), this, SLOT(configurationChanged()) );

        // enable debugging
        QLoggingCategory::setFilterRules(QStringLiteral("breeze.debug = false"));

        // call the slot directly; this initial call will set up things that also
        // need to be reset when the system palette changes
        loadConfiguration();

    }

    //______________________________________________________________
    Style::~Style( void )
    { delete _helper; }

    //______________________________________________________________
    void Style::polish( QWidget* widget )
    {
        if( !widget ) return;

        // register widget to animations
        _animations->registerWidget( widget );
        _windowManager->registerWidget( widget );

        // enable hover effects for all necessary widgets
        if(
            qobject_cast<QAbstractItemView*>( widget )
            || qobject_cast<QAbstractSpinBox*>( widget )
            || qobject_cast<QCheckBox*>( widget )
            || qobject_cast<QComboBox*>( widget )
            || qobject_cast<QDial*>( widget )
            || qobject_cast<QLineEdit*>( widget )
            || qobject_cast<QPushButton*>( widget )
            || qobject_cast<QRadioButton*>( widget )
            || qobject_cast<QScrollBar*>( widget )
            || qobject_cast<QSlider*>( widget )
            || qobject_cast<QSplitterHandle*>( widget )
            || qobject_cast<QTabBar*>( widget )
            || qobject_cast<QTextEdit*>( widget )
            || qobject_cast<QToolButton*>( widget )
            )
        { widget->setAttribute( Qt::WA_Hover ); }

        if( QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>( widget ) )
        {

            // enable hover effects in itemviews' viewport
            itemView->viewport()->setAttribute( Qt::WA_Hover );

        } else if( QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>( widget ) ) {

            // enable hover effect in sunken scrollareas that support focus
            if( scrollArea->frameShadow() == QFrame::Sunken && widget->focusPolicy()&Qt::StrongFocus )
            { widget->setAttribute( Qt::WA_Hover ); }

        } else if( QGroupBox* groupBox = qobject_cast<QGroupBox*>( widget ) )  {

            // checkable group boxes
            if( groupBox->isCheckable() )
            { groupBox->setAttribute( Qt::WA_Hover ); }

        } else if( qobject_cast<QAbstractButton*>( widget ) && qobject_cast<QDockWidget*>( widget->parent() ) ) {

            widget->setAttribute( Qt::WA_Hover );

        } else if( qobject_cast<QAbstractButton*>( widget ) && qobject_cast<QToolBox*>( widget->parent() ) ) {

            widget->setAttribute( Qt::WA_Hover );

        }

        // remove opaque painting for scrollbars
        if( qobject_cast<QScrollBar*>( widget ) )
        {

            widget->setAttribute( Qt::WA_OpaquePaintEvent, false );

        }

        // base class polishing
        KStyle::polish( widget );

    }

    //_______________________________________________________________
    void Style::unpolish( QWidget* widget )
    {

        // register widget to animations
        _windowManager->unregisterWidget( widget );

        KStyle::unpolish( widget );

    }

    //______________________________________________________________
    int Style::pixelMetric( PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
    {

        // handle special cases
        switch( metric )
        {


            case PM_ScrollBarExtent: return Metrics::ScrollBar_Extend;
            case PM_ScrollBarSliderMin: return Metrics::ScrollBar_MinSliderHeight;

            // fallback
            default: return KStyle::pixelMetric( metric, option, widget );

        }

    }

    //______________________________________________________________
    int Style::styleHint( StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData ) const
    {
        switch( hint )
        {

            case SH_ScrollBar_MiddleClickAbsolutePosition: return true;

            // fallback
            default: return KStyle::styleHint( hint, option, widget, returnData );

        }

    }

    //______________________________________________________________
    QRect Style::subElementRect( SubElement element, const QStyleOption* option, const QWidget* widget ) const
    {
        switch( element )
        {

            // fallback
            default: return KStyle::subElementRect( element, option, widget );

        }
    }

    //______________________________________________________________
    QRect Style::subControlRect( ComplexControl element, const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {
        switch( element )
        {

            case CC_ScrollBar: return scrollBarSubControlRect( option, subControl, widget );

            // fallback
            default: return KStyle::subControlRect( element, option, subControl, widget );

        }

    }

    //______________________________________________________________
    QSize Style::sizeFromContents( ContentsType element, const QStyleOption* option, const QSize& size, const QWidget* widget ) const
    {

        switch( element )
        {

            // fallback
            default: return KStyle::sizeFromContents( element, option, size, widget );

        }

    }

    //______________________________________________________________
    QStyle::SubControl Style::hitTestComplexControl( ComplexControl control, const QStyleOptionComplex* option, const QPoint& point, const QWidget* widget ) const
    {
        switch( control )
        {
            case CC_ScrollBar:
            {

                QRect groove = scrollBarSubControlRect( option, SC_ScrollBarGroove, widget );
                if ( groove.contains( point ) )
                {
                    //Must be either page up/page down, or just click on the slider.
                    //Grab the slider to compare
                    QRect slider = scrollBarSubControlRect( option, SC_ScrollBarSlider, widget );

                    if( slider.contains( point ) ) return SC_ScrollBarSlider;
                    else if( preceeds( point, slider, option ) ) return SC_ScrollBarSubPage;
                    else return SC_ScrollBarAddPage;

                }

                //This is one of the up/down buttons. First, decide which one it is.
                if( preceeds( point, groove, option ) )
                {

                    if( _subLineButtons == DoubleButton )
                    {
                        QRect buttonRect = scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine );
                        return scrollBarHitTest( buttonRect, point, option );
                    } else return SC_ScrollBarSubLine;

                }

                if( _addLineButtons == DoubleButton )
                {

                    QRect buttonRect = scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine );
                    return scrollBarHitTest( buttonRect, point, option );

                } else return SC_ScrollBarAddLine;
            }

            // fallback
            default: return KStyle::hitTestComplexControl( control, option, point, widget );

        }

    }

    //______________________________________________________________
    void Style::drawPrimitive( PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        switch( element )
        {

            // fallback
            default: return KStyle::drawPrimitive( element, option, painter, widget );

        }

    }

    //______________________________________________________________
    void Style::drawControl( ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        StyleControl fcn( nullptr );
        switch( element )
        {

            // scrollbars
            case CE_ScrollBarSlider: fcn = &Style::drawScrollBarSliderControl; break;
            case CE_ScrollBarAddLine: fcn = &Style::drawScrollBarAddLineControl; break;
            case CE_ScrollBarAddPage: fcn = &Style::drawScrollBarAddPageControl; break;
            case CE_ScrollBarSubLine: fcn = &Style::drawScrollBarSubLineControl; break;
            case CE_ScrollBarSubPage: fcn = &Style::drawScrollBarSubPageControl; break;

            // fallback
            default: break;

        }

        painter->save();

        // call function if implemented
        if( !( fcn && ( this->*fcn )( option, painter, widget ) ) )
        { KStyle::drawControl( element, option, painter, widget ); }

        painter->restore();

    }

    //______________________________________________________________
    void Style::drawComplexControl( ComplexControl element, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget ) const
    {

        switch( element )
        {

            // fallback
            default: return KStyle::drawComplexControl( element, option, painter, widget );

        }

    }


    //___________________________________________________________________________________
    void Style::drawItemText(
        QPainter* painter, const QRect& r, int flags, const QPalette& palette, bool enabled,
        const QString &text, QPalette::ColorRole textRole ) const
    {

        // hide mnemonics if requested
        if( !_mnemonics->enabled() && ( flags & Qt::TextShowMnemonic ) && !( flags&Qt::TextHideMnemonic ) )
        {
            flags &= ~Qt::TextShowMnemonic;
            flags |= Qt::TextHideMnemonic;
        }

        // fallback
        return KStyle::drawItemText( painter, r, flags, palette, enabled, text, textRole );

    }


    //_____________________________________________________________________
    bool Style::eventFilter( QObject *object, QEvent *event )
    {
        // fallback
        return KStyle::eventFilter( object, event );
    }

    //_____________________________________________________________________
    void Style::configurationChanged()
    {
        // reparse breezerc
        StyleConfigData::self()->readConfig();

        // reload configuration
        loadConfiguration();
    }

    //_____________________________________________________________________
    void Style::loadConfiguration()
    {

        // load helper configuration
        _helper->loadConfig();

        // update caches size
        const int cacheSize( StyleConfigData::cacheEnabled() ? StyleConfigData::maxCacheSize():0 );
        _helper->setMaxCacheSize( cacheSize );

        // reinitialize engines
        _animations->setupEngines();
        _windowManager->initialize();

        // mnemonics
        _mnemonics->setMode( StyleConfigData::mnemonicsMode() );
    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarInternalSubControlRect( const QStyleOptionComplex* option, SubControl subControl ) const
    {

        const QRect& r = option->rect;
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            {
                int majorSize( scrollBarButtonHeight( _subLineButtons ) );
                if( horizontal ) return handleRTL( option, QRect( r.x(), r.y(), majorSize, r.height() ) );
                else return handleRTL( option, QRect( r.x(), r.y(), r.width(), majorSize ) );

            }

            case SC_ScrollBarAddLine:
            {
                int majorSize( scrollBarButtonHeight( _addLineButtons ) );
                if( horizontal ) return handleRTL( option, QRect( r.right() - majorSize, r.y(), majorSize, r.height() ) );
                else return handleRTL( option, QRect( r.x(), r.bottom() - majorSize, r.width(), majorSize ) );
            }

            default: return QRect();

        }
    }

    //___________________________________________________________________________________________________________________
    QRect Style::scrollBarSubControlRect( const QStyleOptionComplex* option, SubControl subControl, const QWidget* widget ) const
    {
        const State& flags( option->state );
        const bool horizontal( flags&State_Horizontal );

        switch ( subControl )
        {

            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            return scrollBarInternalSubControlRect( option, subControl );

            //The main groove area. This is used to compute the others...
            case SC_ScrollBarGroove:
            {
                QRect top = handleRTL( option, scrollBarInternalSubControlRect( option, SC_ScrollBarSubLine ) );
                QRect bot = handleRTL( option, scrollBarInternalSubControlRect( option, SC_ScrollBarAddLine ) );

                QPoint topLeftCorner;
                QPoint botRightCorner;

                if( horizontal )
                {

                    topLeftCorner  = QPoint( top.right() + 1, top.top() );
                    botRightCorner = QPoint( bot.left()  - 1, top.bottom() );

                } else {

                    topLeftCorner  = QPoint( top.left(),  top.bottom() + 1 );
                    botRightCorner = QPoint( top.right(), bot.top()    - 1 );

                }

                // define rect
                return handleRTL( option, QRect( topLeftCorner, botRightCorner )  );

            }

            case SC_ScrollBarSlider:
            {
                const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
                if( !sliderOption ) return QRect();

                //We do handleRTL here to unreflect things if need be
                QRect groove = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if ( sliderOption->minimum == sliderOption->maximum ) return groove;

                //Figure out how much room we have..
                int space( horizontal ? groove.width() : groove.height() );

                //Calculate the portion of this space that the slider should take up.
                int sliderSize = space * qreal( sliderOption->pageStep ) / ( sliderOption->maximum - sliderOption->minimum + sliderOption->pageStep );
                sliderSize = qMax( sliderSize, static_cast<int>(Metrics::ScrollBar_MinSliderHeight ) );
                sliderSize = qMin( sliderSize, space );

                space -= sliderSize;
                if( space <= 0 ) return groove;

                int pos = qRound( qreal( sliderOption->sliderPosition - sliderOption->minimum )/ ( sliderOption->maximum - sliderOption->minimum )*space );
                if( sliderOption->upsideDown ) pos = space - pos;
                if( horizontal ) return handleRTL( option, QRect( groove.x() + pos, groove.y(), sliderSize, groove.height() ) );
                else return handleRTL( option, QRect( groove.x(), groove.y() + pos, groove.width(), sliderSize ) );
            }

            case SC_ScrollBarSubPage:
            {

                //We do handleRTL here to unreflect things if need be
                QRect slider = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarSlider, widget ) );
                QRect groove = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return handleRTL( option, QRect( groove.x(), groove.y(), slider.x() - groove.x(), groove.height() ) );
                else return handleRTL( option, QRect( groove.x(), groove.y(), groove.width(), slider.y() - groove.y() ) );
            }

            case SC_ScrollBarAddPage:
            {

                //We do handleRTL here to unreflect things if need be
                QRect slider = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarSlider, widget ) );
                QRect groove = handleRTL( option, scrollBarSubControlRect( option, SC_ScrollBarGroove, widget ) );

                if( horizontal ) return handleRTL( option, QRect( slider.right() + 1, groove.y(), groove.right() - slider.right(), groove.height() ) );
                else return handleRTL( option, QRect( groove.x(), slider.bottom() + 1, groove.width(), groove.bottom() - slider.bottom() ) );

            }

            default: return QRect();
        }
    }


    //___________________________________________________________________________________
    bool Style::drawScrollBarSliderControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        Q_UNUSED( widget );

        // cast option and check
        const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>( option );
        if( !sliderOption ) return true;

        painter->setClipRect( option->rect );

        // define handle rect
        QRect handleRect;
        const State& flags( option->state );
        const bool horizontal( flags & State_Horizontal );
        if( horizontal ) handleRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
        else handleRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );

        const bool enabled( flags&State_Enabled );
        const bool mouseOver( enabled && ( flags&State_MouseOver ) );

        // enable animation state
        _animations->scrollBarEngine().updateState( widget, enabled && ( sliderOption->activeSubControls & SC_ScrollBarSlider ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, SC_ScrollBarSlider ) );
        if( widget )
        {
            // render background
            // it is necessary to retrive the complete widget rect, in order to properly handle overlaps
            // at the scrollbar boundary
            // define color
            const QPalette& palette( option->palette );
            const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );

            // adjust rect
            QStyleOptionSlider copy( *sliderOption );
            copy.rect = widget->rect();
            QRect backgroundRect( scrollBarSubControlRect( &copy, SC_ScrollBarGroove, widget ) );
            if( horizontal ) backgroundRect = centerRect( backgroundRect, backgroundRect.width(), Metrics::ScrollBar_SliderWidth );
            else backgroundRect = centerRect( backgroundRect, Metrics::ScrollBar_SliderWidth, backgroundRect.height() );

            _helper->scrollBarHole( color )->render( backgroundRect, painter, TileSet::Full );

        }

        {
            // render handle
            // define colors
            QColor color;
            const QPalette& palette( option->palette );

            const QColor base( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.5 ) );
            const QColor highlight( palette.color( QPalette::Highlight ) );
            if( opacity >= 0 ) color = KColorUtils::mix( base, highlight, opacity );
            else if( mouseOver ) color = palette.color( QPalette::Highlight );
            else {

                color = _helper->alphaColor( palette.color( QPalette::WindowText ), 0.5 );

            }

            // render
            _helper->scrollBarHandle( color, QColor() )->render( handleRect, painter, TileSet::Full );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarAddLineControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // do nothing if no buttons are defined
        if( _addLineButtons == NoButton ) return true;

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        if( false )
        {
            painter->save();
            painter->setPen( Qt::red );
            painter->setBrush( Qt::NoBrush );
            painter->setRenderHint( QPainter::Antialiasing );
            painter->drawRect( QRectF( option->rect ).adjusted( 0.5, 0.5, -0.5, -0.5 ) );
            painter->restore();
        }

        const State& flags( option->state );
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // colors
        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );

        // adjust rect, based on number of buttons to be drawn
        QRect r( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarAddLine ) );

        QColor color;
        QStyleOptionSlider localOption( *sliderOption );
        if( _addLineButtons == DoubleButton )
        {

            if( horizontal )
            {

                //Draw the arrows
                const QSize halfSize( r.width()/2, r.height() );
                const QRect leftSubButton( r.topLeft(), halfSize );
                const QRect rightSubButton( leftSubButton.topRight() + QPoint( 1, 0 ), halfSize );

                localOption.rect = leftSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarAddLine:SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, leftSubButton, color, background, ArrowLeft );

                localOption.rect = rightSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarSubLine:SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, rightSubButton, color, background, ArrowRight );

            } else {

                const QSize halfSize( r.width(), r.height()/2 );
                const QRect topSubButton( r.topLeft(), halfSize );
                const QRect botSubButton( topSubButton.bottomLeft() + QPoint( 0, 1 ), halfSize );

                localOption.rect = topSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, topSubButton, color, background, ArrowUp );

                localOption.rect = botSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, botSubButton, color, background, ArrowDown );

            }

        } else if( _addLineButtons == SingleButton ) {

            localOption.rect = r;
            color = scrollBarArrowColor( &localOption,  SC_ScrollBarAddLine, widget );
            if( horizontal )
            {

                if( reverseLayout ) renderScrollBarArrow( painter, r, color, background, ArrowLeft );
                else renderScrollBarArrow( painter, r.translated( 1, 0 ), color, background, ArrowRight );

            } else renderScrollBarArrow( painter, r.translated( 0, 1 ), color, background, ArrowDown );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarAddPageControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        painter->setClipRect( option->rect );

        const QPalette& palette( option->palette );
        const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
        const State& flags( option->state );

        // define tiles and adjust rect
        TileSet::Tiles tiles;
        QRect backgroundRect;
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( sliderOption->direction == Qt::RightToLeft );

        if( horizontal )
        {

            backgroundRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
            tiles = TileSet::Vertical;
            if( reverseLayout )
            {
                tiles |= TileSet::Left;
                backgroundRect.adjust( 0, 0, Metrics::ScrollBar_SliderWidth/2, 0 );

            } else {

                tiles |= TileSet::Right;
                backgroundRect.adjust( -Metrics::ScrollBar_SliderWidth/2, 0, 0, 0 );

            }


        } else {

            backgroundRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );
            tiles = TileSet::Horizontal|TileSet::Bottom;
            backgroundRect.adjust( 0, -Metrics::ScrollBar_SliderWidth/2, 0, 0 );

        }

        // render
        _helper->scrollBarHole( color )->render( backgroundRect, painter, tiles );

        return true;

    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarSubLineControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
    {

        // do nothing if no buttons are set
        if( _subLineButtons == NoButton ) return true;

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        if( false )
        {
            painter->save();
            painter->setPen( Qt::red );
            painter->setBrush( Qt::NoBrush );
            painter->setRenderHint( QPainter::Antialiasing );
            painter->drawRect( QRectF( option->rect ).adjusted( 0.5, 0.5, -0.5, -0.5 ) );
            painter->restore();
        }

        const State& flags( option->state );
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( option->direction == Qt::RightToLeft );

        // colors
        const QPalette& palette( option->palette );
        const QColor background( palette.color( QPalette::Window ) );

        // adjust rect, based on number of buttons to be drawn
        QRect r( scrollBarInternalSubControlRect( sliderOption, SC_ScrollBarSubLine ) );

        QColor color;
        QStyleOptionSlider localOption( *sliderOption );
        if( _subLineButtons == DoubleButton )
        {

            if( horizontal )
            {

                //Draw the arrows
                const QSize halfSize( r.width()/2, r.height() );
                const QRect leftSubButton( r.topLeft(), halfSize );
                const QRect rightSubButton( leftSubButton.topRight() + QPoint( 1, 0 ), halfSize );

                localOption.rect = leftSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarAddLine:SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, leftSubButton, color, background, ArrowLeft );

                localOption.rect = rightSubButton;
                color = scrollBarArrowColor( &localOption,  reverseLayout ? SC_ScrollBarSubLine:SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, rightSubButton, color, background, ArrowRight );

            } else {

                const QSize halfSize( r.width(), r.height()/2 );
                const QRect topSubButton( r.topLeft(), halfSize );
                const QRect botSubButton( topSubButton.bottomLeft() + QPoint( 0, 1 ), halfSize );

                localOption.rect = topSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarSubLine, widget );
                renderScrollBarArrow( painter, topSubButton, color, background, ArrowUp );

                localOption.rect = botSubButton;
                color = scrollBarArrowColor( &localOption, SC_ScrollBarAddLine, widget );
                renderScrollBarArrow( painter, botSubButton, color, background, ArrowDown );

            }

        } else if( _subLineButtons == SingleButton ) {

            localOption.rect = r;
            color = scrollBarArrowColor( &localOption,  SC_ScrollBarSubLine, widget );
            if( horizontal )
            {

                if( reverseLayout ) renderScrollBarArrow( painter, r.translated( 1, 0 ), color, background, ArrowRight );
                else renderScrollBarArrow( painter, r, color, background, ArrowLeft );

            } else renderScrollBarArrow( painter, r, color, background, ArrowUp );

        }

        return true;
    }

    //___________________________________________________________________________________
    bool Style::drawScrollBarSubPageControl( const QStyleOption* option, QPainter* painter, const QWidget* ) const
    {

        // cast option and check
        const QStyleOptionSlider* sliderOption( qstyleoption_cast<const QStyleOptionSlider*>( option ) );
        if ( !sliderOption ) return true;

        painter->setClipRect( option->rect );

        const QPalette& palette( option->palette );
        const QColor color( _helper->alphaColor( palette.color( QPalette::WindowText ), 0.3 ) );
        const State& flags( option->state );

        // define tiles and adjust rect
        TileSet::Tiles tiles;
        QRect backgroundRect;
        const bool horizontal( flags & State_Horizontal );
        const bool reverseLayout( sliderOption->direction == Qt::RightToLeft );

        if( horizontal )
        {

            backgroundRect = centerRect( option->rect, option->rect.width(), Metrics::ScrollBar_SliderWidth );
            tiles = TileSet::Vertical;
            if( reverseLayout )
            {
                tiles |= TileSet::Right;
                backgroundRect.adjust( -Metrics::ScrollBar_SliderWidth/2, 0, 0, 0 );

            } else {

                tiles |= TileSet::Left;
                backgroundRect.adjust( 0, 0, Metrics::ScrollBar_SliderWidth/2-1, 0 );

            }


        } else {

            backgroundRect = centerRect( option->rect, Metrics::ScrollBar_SliderWidth, option->rect.height() );
            tiles = TileSet::Horizontal|TileSet::Top;
            backgroundRect.adjust( 0, 0, 0, Metrics::ScrollBar_SliderWidth/2-1 );

        }

        // render
        _helper->scrollBarHole( color )->render( backgroundRect, painter, tiles );

        return true;

    }

    //______________________________________________________________________________
    void Style::renderScrollBarArrow(
        QPainter* painter, const QRect& r, const QColor& color, const QColor& background,
        ArrowOrientation orientation ) const
    {

        Q_UNUSED( background );

        const qreal penThickness( 1.5 );
        QPolygonF a( genericArrow( orientation, ArrowNormal ) );

        const QColor base( color );

        painter->save();
        painter->translate( QRectF(r).center() );
        painter->setRenderHint( QPainter::Antialiasing );

        painter->setPen( QPen( base, penThickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
        painter->drawPolyline( a );
        painter->restore();

        return;

    }

    //______________________________________________________________________________
    QColor Style::scrollBarArrowColor( const QStyleOptionSlider* option, const SubControl& control, const QWidget* widget ) const
    {

        Q_UNUSED( widget );

        const QRect& r( option->rect );
        const QPalette& palette( option->palette );
        QColor color( palette.color( QPalette::WindowText ) );

        // check enabled state
        const bool enabled( option->state & State_Enabled );
        if( !enabled ) return color;

        if(
            ( control == SC_ScrollBarSubLine && option->sliderValue == option->minimum ) ||
            ( control == SC_ScrollBarAddLine && option->sliderValue == option->maximum ) )
        {

            // manually disable arrow, to indicate that scrollbar is at limit
            return palette.color( QPalette::Disabled, QPalette::WindowText );

        }

        const bool hover( _animations->scrollBarEngine().isHovered( widget, control ) );
        const bool animated( _animations->scrollBarEngine().isAnimated( widget, control ) );
        const qreal opacity( _animations->scrollBarEngine().opacity( widget, control ) );

        // retrieve mouse position from engine
        QPoint position( hover ? _animations->scrollBarEngine().position( widget ) : QPoint( -1, -1 ) );
        if( hover && r.contains( position ) )
        {
            // need to update the arrow controlRect on fly because there is no
            // way to get it from the styles directly, outside of repaint events
            _animations->scrollBarEngine().setSubControlRect( widget, control, r );
        }


        if( r.intersects(  _animations->scrollBarEngine().subControlRect( widget, control ) ) )
        {

            QColor highlight = _helper->viewHoverBrush().brush( palette ).color();
            if( animated )
            {
                color = KColorUtils::mix( color, highlight, opacity );

            } else if( hover ) {

                color = highlight;

            }

        }

        return color;

    }

    //____________________________________________________________________________________
    QPolygonF Style::genericArrow( Style::ArrowOrientation orientation, Style::ArrowSize size ) const
    {

        QPolygonF a;
        switch( orientation )
        {
            case ArrowUp:
            {
                if( size == ArrowTiny ) a << QPointF( -1.75, 1.125 ) << QPointF( 0.5, -1.125 ) << QPointF( 2.75, 1.125 );
                else if( size == ArrowSmall ) a << QPointF( -2,1.5 ) << QPointF( 0.5, -1.5 ) << QPointF( 3,1.5 );
                else a << QPointF( -4,2 ) << QPointF( 0, -2 ) << QPointF( 4,2 );
                break;
            }

            case ArrowDown:
            {
                if( size == ArrowTiny ) a << QPointF( -1.75, -1.125 ) << QPointF( 0.5, 1.125 ) << QPointF( 2.75, -1.125 );
                else if( size == ArrowSmall ) a << QPointF( -2,-1.5 ) << QPointF( 0.5, 1.5 ) << QPointF( 3,-1.5 );
                else a << QPointF( -4,-2 ) << QPointF( 0, 2 ) << QPointF( 4,-2 );
                break;
            }

            case ArrowLeft:
            {
                if( size == ArrowTiny ) a << QPointF( 1.125, -1.75 ) << QPointF( -1.125, 0.5 ) << QPointF( 1.125, 2.75 );
                else if( size == ArrowSmall ) a << QPointF( 1.5,-2 ) << QPointF( -1.5, 0.5 ) << QPointF( 1.5,3 );
                else a << QPointF( 2, -4 ) << QPointF( -2, 0 ) << QPointF( 2, 4 );
                break;
            }

            case ArrowRight:
            {
                if( size == ArrowTiny ) a << QPointF( -1.125, -1.75 ) << QPointF( 1.125, 0.5 ) << QPointF( -1.125, 2.75 );
                else if( size == ArrowSmall ) a << QPointF( -1.5,-2 ) << QPointF( 1.5, 0.5 ) << QPointF( -1.5,3 );
                else a << QPointF( -2,-4 ) << QPointF( 2, 0 ) << QPointF( -2, 4 );
                break;
            }

            default: break;

        }

        return a;

    }

}
