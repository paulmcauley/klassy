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

#include "breezeframeshadow.h"

#include "breeze.h"

#include <QDebug>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QSplitter>

#include <KColorUtils>

namespace Breeze
{

    //____________________________________________________________________________________
    bool FrameShadowFactory::registerWidget( QWidget* widget, Helper& helper )
    {

        if( !widget ) return false;
        if( isRegistered( widget ) ) return false;

        // check whether widget is a frame, and has the proper shape
        bool accepted = false;

        // cast to frame and check
        QFrame* frame( qobject_cast<QFrame*>( widget ) );
        if( frame )
        {

            // also do not install on QSplitter
            /*
            due to Qt, splitters are set with a frame style that matches the condition below,
            though no shadow should be installed, obviously
            */
            if( qobject_cast<QSplitter*>( widget ) ) return false;

            // further checks on frame shape, and parent
            if( frame->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken) ) accepted = true;

        } else if( widget->inherits( "KTextEditor::View" ) ) accepted = true;

        if( !accepted ) return false;

        // make sure that the widget is not embedded into a KHTMLView
        QWidget* parent( widget->parentWidget() );
        while( parent && !parent->isTopLevel() )
        {
            if( parent->inherits( "KHTMLView" ) ) return false;
            parent = parent->parentWidget();
        }

        // store in set
        _registeredWidgets.insert( widget );

        // catch object destruction
        connect( widget, SIGNAL(destroyed(QObject*)), SLOT(widgetDestroyed(QObject*)) );

        // install shadow
        installShadows( widget, helper );

        return true;

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::unregisterWidget( QWidget* widget )
    {
        if( !isRegistered( widget ) ) return;
        _registeredWidgets.remove( widget );
        removeShadows( widget );
    }

    //____________________________________________________________________________________
    bool FrameShadowFactory::eventFilter( QObject* object, QEvent* event )
    {

        switch( event->type() )
        {
            // TODO: possibly implement ZOrderChange event, to make sure that
            // the shadow is always painted on top
            case QEvent::ZOrderChange:
            {
                raiseShadows( object );
                break;
            }

            default: break;
        }

        return QObject::eventFilter( object, event );

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::installShadows( QWidget* widget, Helper& helper )
    {

        removeShadows(widget);

        widget->installEventFilter(this);

        widget->installEventFilter( &_addEventFilter );
        installShadow( widget, helper, SideTop );
        installShadow( widget, helper, SideBottom );
        widget->removeEventFilter( &_addEventFilter );

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::removeShadows( QWidget* widget )
    {

        widget->removeEventFilter( this );

        const QList<QObject* > children = widget->children();
        foreach( QObject *child, children )
        {
            if( FrameShadow* shadow = qobject_cast<FrameShadow*>(child) )
            {
                shadow->hide();
                shadow->setParent(nullptr);
                shadow->deleteLater();
            }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::updateShadowsGeometry( const QObject* object, QRect rect ) const
    {

        const QList<QObject *> &children = object->children();
        foreach( QObject *child, children )
        {
            if( FrameShadow* shadow = qobject_cast<FrameShadow *>(child) )
            { shadow->updateGeometry( rect ); }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::raiseShadows( QObject* object ) const
    {

        const QList<QObject *> &children = object->children();
        foreach( QObject *child, children )
        {
            if( FrameShadow* shadow = qobject_cast<FrameShadow *>(child) )
            { shadow->raise(); }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::update( QObject* object ) const
    {

        const QList<QObject* > &children = object->children();
        foreach( QObject *child, children )
        {
            if( FrameShadow* shadow = qobject_cast<FrameShadow *>(child) )
            { shadow->update();}
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::updateState( const QWidget* widget, bool focus, bool hover, qreal opacity, AnimationMode mode ) const
    {

        const QList<QObject *> &children = widget->children();
        foreach( QObject *child, children )
        {
            if( FrameShadow* shadow = qobject_cast<FrameShadow *>(child) )
            { shadow->updateState( focus, hover, opacity, mode ); }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::installShadow( QWidget* widget, Helper& helper, Side area ) const
    {
        FrameShadow *shadow(nullptr);
        shadow = new FrameShadow( area, helper );
        shadow->setParent(widget);
        shadow->hide();
    }

    //____________________________________________________________________________________
    void FrameShadowFactory::widgetDestroyed( QObject* object )
    { _registeredWidgets.remove( object ); }

    //____________________________________________________________________________________
    FrameShadow::FrameShadow( Side area, Helper& helper ):
        _helper( helper ),
        _area( area )
    {

        setAttribute(Qt::WA_OpaquePaintEvent, false);

        setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setContextMenuPolicy(Qt::NoContextMenu);

        // grab viewport widget
        QWidget *viewport( this->viewport() );

        // set cursor from viewport
        if (viewport) setCursor(viewport->cursor());

    }

    //____________________________________________________________________________________
    void FrameShadow::updateGeometry( QRect rect )
    {

        // show on first call
        if( isHidden() ) show();

        // store offsets between passed rect and parent widget rect
        QRect parentRect( parentWidget()->contentsRect() );
        _margins = QMargins(
            rect.left() - parentRect.left(),
            rect.top() - parentRect.top(),
            rect.right() - parentRect.right(),
            rect.bottom() - parentRect.bottom() );

        // for efficiency, take out the part for which nothing is rendered
        rect.adjust( 1, 1, -1, -1 );

        // adjust geometry
        const int shadowSize( Metrics::Frame_FrameRadius );
        switch( _area )
        {

            case SideTop:
            rect.setHeight( shadowSize );
            break;

            case SideBottom:
            rect.setTop( rect.bottom() - shadowSize + 1 );
            break;

            case SideLeft:
            rect.setWidth(shadowSize);
            rect.adjust(0, shadowSize, 0, -shadowSize );
            break;


            case SideRight:
            rect.setLeft(rect.right() - shadowSize + 1 );
            rect.adjust(0, shadowSize, 0, -shadowSize );
            break;

            default:
            return;
        }

        setGeometry(rect);

    }

    //____________________________________________________________________________________
    void FrameShadow::updateState( bool focus, bool hover, qreal opacity, AnimationMode mode )
    {
        bool changed( false );
        if( _hasFocus != focus ) { _hasFocus = focus; changed |= true; }
        if( _mouseOver != hover ) { _mouseOver = hover; changed |= !_hasFocus; }
        if( _mode != mode )
        {

            _mode = mode;
            changed |=
                (_mode == AnimationNone) ||
                (_mode == AnimationFocus) ||
                (_mode == AnimationHover && !_hasFocus );

        }

        if( _opacity != opacity ) { _opacity = opacity; changed |= (_mode != AnimationNone ); }
        if( changed )
        {

            if( QWidget* viewport = this->viewport() )
            {

                // need to disable viewport updates to avoid some redundant painting
                // besides it fixes one visual glitch (from Qt) in QTableViews
                viewport->setUpdatesEnabled( false );
                update() ;
                viewport->setUpdatesEnabled( true );

            } else update();

        }
    }

    //____________________________________________________________________________________
    void FrameShadow::paintEvent(QPaintEvent *event )
    {

        // this fixes shadows in frames that change frameStyle() after polish()
        if( QFrame *frame = qobject_cast<QFrame *>( parentWidget() ) )
        { if (frame->frameStyle() != (QFrame::StyledPanel | QFrame::Sunken)) return; }

        const QRect parentRect( parentWidget()->contentsRect().translated( mapFromParent( QPoint( 0, 0 ) ) ) );
        const QRect rect( parentRect.adjusted( _margins.left(), _margins.top(), _margins.right(), _margins.bottom() ) );

        // render
        QPainter painter(this);
        painter.setClipRegion( event->region() );
        painter.setRenderHint( QPainter::Antialiasing );

        const QColor outline( _helper.frameOutlineColor( palette(), _mouseOver, _hasFocus, _opacity, _mode ) );
        painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
        _helper.renderFrame( &painter, rect, QColor(), outline );

    }

    //____________________________________________________________________________________
    QWidget* FrameShadow::viewport() const
    {

        if( !parentWidget() ) return nullptr;
        else if( QAbstractScrollArea *widget = qobject_cast<QAbstractScrollArea *>(parentWidget()) ) {

            return widget->viewport();

        } else return nullptr;

    }

}
