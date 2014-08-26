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
#include "breezeframeshadow.moc"

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
        if( !frame ) return false;

        // also do not install on QSplitter
        /*
        due to Qt, splitters are set with a frame style that matches the condition below,
        though no shadow should be installed, obviously
        */
        if( qobject_cast<QSplitter*>( widget ) ) return false;

        // further checks on frame shape, and parent
        if( frame->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken) ) accepted = true;
        else if( widget->parent() && widget->parent()->inherits( "QComboBoxPrivateContainer" ) ) accepted = true;

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

            case QEvent::Show:
            updateShadowsGeometry( object );
            update( object );
            break;

            case QEvent::Resize:
            updateShadowsGeometry( object );
            break;

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
        installShadow( widget, helper, ShadowAreaTop );
        installShadow( widget, helper, ShadowAreaBottom );
        installShadow( widget, helper, ShadowAreaLeft );
        installShadow( widget, helper, ShadowAreaRight );
        widget->removeEventFilter( &_addEventFilter );

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::removeShadows( QWidget* widget )
    {

        widget->removeEventFilter( this );

        const QList<QObject* > children = widget->children();
        foreach( QObject *child, children )
        {
            if( FrameShadowBase* shadow = qobject_cast<FrameShadowBase*>(child) )
            {
                shadow->hide();
                shadow->setParent(0);
                shadow->deleteLater();
            }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::updateShadowsGeometry( QObject* object ) const
    {

        const QList<QObject *> children = object->children();
        foreach( QObject *child, children )
        {
            if( FrameShadowBase* shadow = qobject_cast<FrameShadowBase *>(child) )
            { shadow->updateGeometry(); }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::raiseShadows( QObject* object ) const
    {

        const QList<QObject *> children = object->children();
        foreach( QObject *child, children )
        {
            if( FrameShadowBase* shadow = qobject_cast<FrameShadowBase *>(child) )
            { shadow->raise(); }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::update( QObject* object ) const
    {

        const QList<QObject* > children = object->children();
        foreach( QObject *child, children )
        {
            if( FrameShadowBase* shadow = qobject_cast<FrameShadowBase *>(child) )
            { shadow->update();}
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::updateState( const QWidget* widget, bool focus, bool hover, qreal opacity, AnimationMode mode ) const
    {

        const QList<QObject *> children = widget->children();
        foreach( QObject *child, children )
        {
            if( FrameShadowBase* shadow = qobject_cast<FrameShadowBase *>(child) )
            { shadow->updateState( focus, hover, opacity, mode ); }
        }

    }

    //____________________________________________________________________________________
    void FrameShadowFactory::installShadow( QWidget* widget, Helper& helper, ShadowArea area ) const
    {
        FrameShadowBase *shadow(0);
        shadow = new SunkenFrameShadow( area, helper );
        shadow->setParent(widget);
        shadow->updateGeometry();
        shadow->show();
    }

    //____________________________________________________________________________________
    void FrameShadowFactory::widgetDestroyed( QObject* object )
    { _registeredWidgets.remove( object ); }

    //____________________________________________________________________________________
    void FrameShadowBase::init()
    {

        setAttribute(Qt::WA_OpaquePaintEvent, false);

        setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setContextMenuPolicy(Qt::NoContextMenu);

        // grab viewport widget
        QWidget *viewport( FrameShadowBase::viewport() );
        if( !viewport && parentWidget() && parentWidget()->inherits( "Q3ListView" ) )
        { viewport = parentWidget(); }

        // set cursor from viewport
        if (viewport) setCursor(viewport->cursor());

    }

     //____________________________________________________________________________________
    QWidget* FrameShadowBase::viewport( void ) const
    {

        if( !parentWidget() )  return NULL;

        if( QAbstractScrollArea *widget = qobject_cast<QAbstractScrollArea *>(parentWidget()) )
        {

            return widget->viewport();

        } else return NULL;

    }

    //____________________________________________________________________________________
    bool FrameShadowBase::event(QEvent *e)
    {

        // paintEvents are handled separately
        if (e->type() == QEvent::Paint) return QWidget::event(e);

        QWidget *viewport( FrameShadowBase::viewport() );

        switch (e->type())
        {

            case QEvent::DragEnter:
            case QEvent::DragMove:
            case QEvent::DragLeave:
            case QEvent::Drop:
            if( viewport )
            {
                setAcceptDrops(viewport->acceptDrops());
                return viewport->QObject::event(e);
            }
            break;

            case QEvent::Enter:
            if( viewport ) {
                setCursor(viewport->cursor());
                setAcceptDrops(viewport->acceptDrops());
            }
            break;

            case QEvent::ContextMenu:
            if( viewport )
            {

                QContextMenuEvent *me = static_cast<QContextMenuEvent *>(e);
                QContextMenuEvent *ne = new QContextMenuEvent(me->reason(), parentWidget()->mapFromGlobal(me->globalPos()), me->globalPos());
                QApplication::sendEvent(viewport, ne);
                e->accept();
                return true;
            }
            break;

            case QEvent::MouseButtonPress: releaseMouse();
            case QEvent::MouseMove:
            case QEvent::MouseButtonRelease:
            if( viewport )
            {
                QMouseEvent *me = static_cast<QMouseEvent *>(e);
                QMouseEvent *ne = new QMouseEvent(e->type(), parentWidget()->mapFromGlobal(me->globalPos()), me->globalPos(), me->button(), me->buttons(), me->modifiers());
                QApplication::sendEvent(viewport, ne);
                e->accept();
                return true;
            }
            break;

            default:
            break;
        }

        e->ignore();
        return false;

    }

    //____________________________________________________________________________________
    void SunkenFrameShadow::updateGeometry()
    {

        QWidget *widget = parentWidget();
        if( !widget ) return;

        QRect cr = widget->contentsRect();
        switch (shadowArea())
        {

            case ShadowAreaTop:
            cr.setHeight( ShadowSizeTop );
            cr.adjust( -1, -1, 1, 0 );
            break;


            case ShadowAreaBottom:
            cr.setTop(cr.bottom() - ShadowSizeBottom + 1);
            cr.adjust( -1, 0, 1, 1 );
            break;

            case ShadowAreaLeft:
            cr.setWidth(ShadowSizeLeft);
            cr.adjust(-1, ShadowSizeTop, 0, -ShadowSizeBottom);
            break;


            case ShadowAreaRight:
            cr.setLeft(cr.right() - ShadowSizeRight + 1);
            cr.adjust(0, ShadowSizeTop, 1, -ShadowSizeBottom);
            break;

            default:
            return;
        }

        setGeometry(cr);
    }

    //____________________________________________________________________________________
    void SunkenFrameShadow::updateState( bool focus, bool hover, qreal opacity, AnimationMode mode )
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
    void SunkenFrameShadow::paintEvent(QPaintEvent *event )
    {

        // this fixes shadows in frames that change frameStyle() after polish()
        if (QFrame *frame = qobject_cast<QFrame *>(parentWidget()))
        { if (frame->frameStyle() != (QFrame::StyledPanel | QFrame::Sunken)) return; }

        QWidget *parent = parentWidget();
        if(!parent) return;

        QRect rect = parent->contentsRect();
        rect.translate(mapFromParent(QPoint(0, 0)));

        const int frameWidth( Metrics::Frame_FrameWidth );
        switch( shadowArea() )
        {
            case ShadowAreaTop: rect.adjust( -frameWidth, -frameWidth, frameWidth, -1 ); break;
            case ShadowAreaBottom: rect.adjust( -frameWidth, 1, frameWidth, frameWidth ); break;
            case ShadowAreaLeft: rect.adjust( -frameWidth, -2*frameWidth, -1, 2*frameWidth ); break;
            case ShadowAreaRight: rect.adjust( -1, -2*frameWidth, frameWidth, 2*frameWidth ); break;
            default: return;
        }

        // render
        QPainter painter(this);
        painter.setClipRegion( event->region() );
        painter.setRenderHint( QPainter::Antialiasing );

        {
            // render frame
            const QColor outline( _helper.frameOutlineColor( palette(), _mouseOver, _hasFocus, _opacity, _mode ) );
            painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
            _helper.renderFrame( &painter, rect, QColor(), outline, _hasFocus );
        }

        return;

    }

}
