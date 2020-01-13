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

//////////////////////////////////////////////////////////////////////////////
// breezewindowmanager.cpp
// pass some window mouse press/release/move event actions to window manager
// -------------------
//
// Copyright (c) 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Largely inspired from BeSpin style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "breezewindowmanager.h"
#include "breezepropertynames.h"
#include "breezehelper.h"

#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QGraphicsView>
#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMouseEvent>
#include <QProgressBar>
#include <QScreen>
#include <QScrollBar>
#include <QStatusBar>
#include <QStyle>
#include <QStyleOptionGroupBox>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>

#include <QTextStream>

// needed to deal with device pixel ratio
#include <QWindow>

#if BREEZE_HAVE_QTQUICK
// needed to enable dragging from QQuickWindows
#include <QQuickItem>
#include <QQuickWindow>
#endif

#if BREEZE_HAVE_X11
#include <QX11Info>
#include <xcb/xcb.h>

#include <NETWM>

#endif

#if BREEZE_HAVE_KWAYLAND
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/pointer.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/shell.h>
#include <KWayland/Client/seat.h>
#endif

namespace Util
{
    template<class T>
        inline T makeT( std::initializer_list<typename T::key_type>&& reference )
    {
        return T( std::move( reference ) );
    }
}

namespace Breeze
{

    //* provide application-wise event filter
    /**
    it us used to unlock dragging and make sure event look is properly restored
    after a drag has occurred
    */
    class AppEventFilter: public QObject
    {

        public:

        //* constructor
        explicit AppEventFilter( WindowManager* parent ):
            QObject( parent ),
            _parent( parent )
        {}

        //* event filter
        bool eventFilter( QObject* object, QEvent* event ) override
        {

            if( event->type() == QEvent::MouseButtonRelease )
            {

                // stop drag timer
                if( _parent->_dragTimer.isActive() )
                { _parent->resetDrag(); }

                // unlock
                if( _parent->isLocked() )
                { _parent->setLocked( false ); }

            }

            if( !_parent->enabled() ) return false;

            /*
            if a drag is in progress, the widget will not receive any event
            we trigger on the first MouseMove or MousePress events that are received
            by any widget in the application to detect that the drag is finished
            */
            if( _parent->useWMMoveResize() && _parent->_dragInProgress && _parent->_target && ( event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress ) )
            { return appMouseEvent( object, event ); }

            return false;

        }

        protected:

        //* application-wise event.
        /** needed to catch end of XMoveResize events */
        bool appMouseEvent( QObject*, QEvent* event )
        {

            Q_UNUSED( event );

            /*
            post some mouseRelease event to the target, in order to counter balance
            the mouse press that triggered the drag. Note that it triggers a resetDrag
            */
            QMouseEvent mouseEvent( QEvent::MouseButtonRelease, _parent->_dragPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
            qApp->sendEvent( _parent->_target.data(), &mouseEvent );

            return false;

        }

        private:

        //* parent
        WindowManager* _parent = nullptr;

    };

    //_____________________________________________________________
    WindowManager::WindowManager( QObject* parent ):
        QObject( parent )
    {

        // install application wise event filter
        _appEventFilter = new AppEventFilter( this );
        qApp->installEventFilter( _appEventFilter );

    }

    //_____________________________________________________________
    void WindowManager::initialize()
    {

        setEnabled( StyleConfigData::windowDragMode() != StyleConfigData::WD_NONE );
        setDragMode( StyleConfigData::windowDragMode() );
        setUseWMMoveResize( StyleConfigData::useWMMoveResize() );

        setDragDistance( QApplication::startDragDistance() );
        setDragDelay( QApplication::startDragTime() );

        initializeWhiteList();
        initializeBlackList();
        initializeWayland();

    }

    //_______________________________________________________
    void WindowManager::initializeWayland()
    {
        #if BREEZE_HAVE_KWAYLAND
        if( !Helper::isWayland() ) return;
        if( _seat )  return;

        using namespace KWayland::Client;
        auto connection = ConnectionThread::fromApplication( this );
        if( !connection ) return;

        auto registry = new Registry( this );
        registry->create( connection );
        connect(registry, &Registry::interfacesAnnounced, this,
            [registry, this] {
                const auto interface = registry->interface( Registry::Interface::Seat );
                if( interface.name != 0 )
                {
                    _seat = registry->createSeat( interface.name, interface.version, this );
                    connect(_seat, &Seat::hasPointerChanged, this, &WindowManager::waylandHasPointerChanged);
                }
            }
        );

        registry->setup();
        connection->roundtrip();
        #endif
    }

    //_______________________________________________________
    void WindowManager::waylandHasPointerChanged(bool hasPointer)
    {
        #if BREEZE_HAVE_KWAYLAND
        Q_ASSERT( _seat );
        if( hasPointer )
        {
            if( !_pointer )
            {
                _pointer = _seat->createPointer(this);
                connect(_pointer, &KWayland::Client::Pointer::buttonStateChanged, this,
                    [this] (quint32 serial) { _waylandSerial = serial; }
                );
            }
        } else {
            delete _pointer;
            _pointer = nullptr;
        }
        #else
        Q_UNUSED( hasPointer );
        #endif
    }

    //_____________________________________________________________
    void WindowManager::registerWidget( QWidget* widget )
    {

        if( isBlackListed( widget ) || isDragable( widget ) )
        {

            /*
            install filter for dragable widgets.
            also install filter for blacklisted widgets
            to be able to catch the relevant events and prevent
            the drag to happen
            */
            widget->removeEventFilter( this );
            widget->installEventFilter( this );

        }

    }

    #if BREEZE_HAVE_QTQUICK
    //_____________________________________________________________
    void WindowManager::registerQuickItem( QQuickItem* item )
    {
        if ( !item ) return;

        if( auto window = item->window() )
        {
            auto contentItem = window->contentItem();
            contentItem->setAcceptedMouseButtons( Qt::LeftButton );
            contentItem->removeEventFilter( this );
            contentItem->installEventFilter( this );
        }

    }
    #endif

    //_____________________________________________________________
    void WindowManager::unregisterWidget( QWidget* widget )
    { if( widget ) widget->removeEventFilter( this ); }

    //_____________________________________________________________
    void WindowManager::initializeWhiteList()
    {

        _whiteList = Util::makeT<ExceptionSet>({
            ExceptionId( QStringLiteral( "MplayerWindow" ) ),
            ExceptionId( QStringLiteral( "ViewSliders@kmix" ) ),
            ExceptionId( QStringLiteral( "Sidebar_Widget@konqueror" ) )
        });

        foreach( const QString& exception, StyleConfigData::windowDragWhiteList() )
        {
            ExceptionId id( exception );
            if( !id.className().isEmpty() )
            { _whiteList.insert( ExceptionId( exception ) ); }
        }
    }

    //_____________________________________________________________
    void WindowManager::initializeBlackList()
    {

        _blackList = Util::makeT<ExceptionSet>({
            ExceptionId( QStringLiteral( "CustomTrackView@kdenlive" ) ),
            ExceptionId( QStringLiteral( "MuseScore" ) ),
            ExceptionId( QStringLiteral( "KGameCanvasWidget" ) )
        });

        foreach( const QString& exception, StyleConfigData::windowDragBlackList() )
        {
            ExceptionId id( exception );
            if( !id.className().isEmpty() )
            { _blackList.insert( ExceptionId( exception ) ); }
        }

    }

    //_____________________________________________________________
    bool WindowManager::eventFilter( QObject* object, QEvent* event )
    {
        if( !enabled() ) return false;

        switch ( event->type() )
        {
            case QEvent::MouseButtonPress:
            return mousePressEvent( object, event );
            break;

            case QEvent::MouseMove:
            if ( object == _target.data()
                #if BREEZE_HAVE_QTQUICK
                || object == _quickTarget.data()
                #endif
               ) return mouseMoveEvent( object, event );
            break;

            case QEvent::MouseButtonRelease:
            if ( _target
                #if BREEZE_HAVE_QTQUICK
                || _quickTarget
                #endif
               ) return mouseReleaseEvent( object, event );
            break;

            default:
            break;

        }

        return false;

    }

    //_____________________________________________________________
    void WindowManager::timerEvent( QTimerEvent* event )
    {

        if( event->timerId() == _dragTimer.timerId() )
        {

            _dragTimer.stop();
            if( _target ) startDrag( _target.data()->window()->windowHandle(), _globalDragPoint );
            #if BREEZE_HAVE_QTQUICK
            else if( _quickTarget ) startDrag( _quickTarget.data()->window(), _globalDragPoint );
            #endif

        } else {

            return QObject::timerEvent( event );

        }

    }

    //_____________________________________________________________
    bool WindowManager::mousePressEvent( QObject* object, QEvent* event )
    {

        // cast event and check buttons/modifiers
        auto mouseEvent = static_cast<QMouseEvent*>( event );
        if (mouseEvent->source() != Qt::MouseEventNotSynthesized)
        { return false; }
        if( !( mouseEvent->modifiers() == Qt::NoModifier && mouseEvent->button() == Qt::LeftButton ) )
        { return false; }

        // check lock
        if( isLocked() ) return false;
        else setLocked( true );

        #if BREEZE_HAVE_QTQUICK
        // check QQuickItem - we can immediately start drag, because QQuickWindow's contentItem
        // only receives mouse events that weren't handled by children
        if( auto item = qobject_cast<QQuickItem*>( object ) )
        {
            _quickTarget = item;
            _dragPoint = mouseEvent->pos();
            _globalDragPoint = mouseEvent->globalPos();

            if( _dragTimer.isActive() ) _dragTimer.stop();
            _dragTimer.start( _dragDelay, this );

            return true;
        }
        #endif

        // cast to widget
        auto widget = static_cast<QWidget*>( object );

        // check if widget can be dragged from current position
        if( isBlackListed( widget ) || !canDrag( widget ) ) return false;

        // retrieve widget's child at event position
        auto position( mouseEvent->pos() );
        auto child = widget->childAt( position );
        if( !canDrag( widget, child, position ) ) return false;

        // save target and drag point
        _target = widget;
        _dragPoint = position;
        _globalDragPoint = mouseEvent->globalPos();
        _dragAboutToStart = true;

        // send a move event to the current child with same position
        // if received, it is caught to actually start the drag
        auto localPoint( _dragPoint );
        if( child ) localPoint = child->mapFrom( widget, localPoint );
        else child = widget;
        QMouseEvent localMouseEvent( QEvent::MouseMove, localPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
        qApp->sendEvent( child, &localMouseEvent );

        // never eat event
        return false;

    }

    //_____________________________________________________________
    bool WindowManager::mouseMoveEvent( QObject* object, QEvent* event )
    {

        Q_UNUSED( object );

        // stop timer
        if( _dragTimer.isActive() ) _dragTimer.stop();

        // cast event and check drag distance
        auto mouseEvent = static_cast<QMouseEvent*>( event );
        if (mouseEvent->source() != Qt::MouseEventNotSynthesized)
        { return false; }
        if( !_dragInProgress )
        {

            if( _dragAboutToStart )
            {
                if( mouseEvent->pos() == _dragPoint )
                {
                    // start timer,
                    _dragAboutToStart = false;
                    if( _dragTimer.isActive() ) _dragTimer.stop();
                    _dragTimer.start( _dragDelay, this );

                } else resetDrag();

            } else if( QPoint( mouseEvent->globalPos() - _globalDragPoint ).manhattanLength() >= _dragDistance ) {

                _dragTimer.start( 0, this );

            }

            return true;

        } else if( !useWMMoveResize() && _target ) {

            // use QWidget::move for the grabbing
            /* this works only if the sending object and the target are identical */
            auto window( _target.data()->window() );
            window->move( window->pos() + mouseEvent->pos() - _dragPoint );
            return true;

        } else return false;

    }

    //_____________________________________________________________
    bool WindowManager::mouseReleaseEvent( QObject* object, QEvent* event )
    {

        Q_UNUSED( object );
        Q_UNUSED( event );
        resetDrag();
        return false;
    }

    //_____________________________________________________________
    bool WindowManager::isDragable( QWidget* widget )
    {

        // check widget
        if( !widget ) return false;

        // accepted default types
        if(
            ( qobject_cast<QDialog*>( widget ) && widget->isWindow() ) ||
            ( qobject_cast<QMainWindow*>( widget ) && widget->isWindow() ) ||
            qobject_cast<QGroupBox*>( widget ) )
        { return true; }

        // more accepted types, provided they are not dock widget titles
        if( ( qobject_cast<QMenuBar*>( widget ) ||
            qobject_cast<QTabBar*>( widget ) ||
            qobject_cast<QStatusBar*>( widget ) ||
            qobject_cast<QToolBar*>( widget ) ) &&
            !isDockWidgetTitle( widget ) )
        { return true; }

        if( widget->inherits( "KScreenSaver" ) && widget->inherits( "KCModule" ) )
        { return true; }

        if( isWhiteListed( widget ) )
        { return true; }

        // flat toolbuttons
        if( auto toolButton = qobject_cast<QToolButton*>( widget ) )
        { if( toolButton->autoRaise() ) return true; }

        // viewports
        /*
        one needs to check that
        1/ the widget parent is a scrollarea
        2/ it matches its parent viewport
        3/ the parent is not blacklisted
        */
        if( auto listView = qobject_cast<QListView*>( widget->parentWidget() ) )
        { if( listView->viewport() == widget && !isBlackListed( listView ) ) return true; }

        if( auto treeView = qobject_cast<QTreeView*>( widget->parentWidget() ) )
        { if( treeView->viewport() == widget && !isBlackListed( treeView ) ) return true; }

        /*
        catch labels in status bars.
        this is because of kstatusbar
        who captures buttonPress/release events
        */
        if( auto label = qobject_cast<QLabel*>( widget ) )
        {
            if( label->textInteractionFlags().testFlag( Qt::TextSelectableByMouse ) ) return false;

            QWidget* parent = label->parentWidget();
            while( parent )
            {
                if( qobject_cast<QStatusBar*>( parent ) ) return true;
                parent = parent->parentWidget();
            }
        }

        return false;

    }

    //_____________________________________________________________
    bool WindowManager::isBlackListed( QWidget* widget )
    {

        // check against noAnimations property
        const auto propertyValue( widget->property( PropertyNames::noWindowGrab ) );
        if( propertyValue.isValid() && propertyValue.toBool() ) return true;

        // list-based blacklisted widgets
        const auto appName( qApp->applicationName() );
        foreach( const ExceptionId& id, _blackList )
        {
            if( !id.appName().isEmpty() && id.appName() != appName ) continue;
            if( id.className() == QStringLiteral( "*" ) && !id.appName().isEmpty() )
            {
                // if application name matches and all classes are selected
                // disable the grabbing entirely
                setEnabled( false );
                return true;
            }
            if( widget->inherits( id.className().toLatin1().data() ) ) return true;
        }

        return false;
    }

    //_____________________________________________________________
    bool WindowManager::isWhiteListed( QWidget* widget ) const
    {

        const auto appName( qApp->applicationName() );
        foreach( const ExceptionId& id, _whiteList )
        {
            if( !(id.appName().isEmpty() || id.appName() == appName ) ) continue;
            if( widget->inherits( id.className().toLatin1().data() ) ) return true;
        }

        return false;
    }

    //_____________________________________________________________
    bool WindowManager::canDrag( QWidget* widget )
    {

        // check if enabled
        if( !enabled() ) return false;

        // assume isDragable widget is already passed
        // check some special cases where drag should not be effective

        // check mouse grabber
        if( QWidget::mouseGrabber() ) return false;

        /*
        check cursor shape.
        Assume that a changed cursor means that some action is in progress
        and should prevent the drag
        */
        if( widget->cursor().shape() != Qt::ArrowCursor ) return false;

        // accept
        return true;

    }

    //_____________________________________________________________
    bool WindowManager::canDrag( QWidget* widget, QWidget* child, const QPoint& position )
    {

        // retrieve child at given position and check cursor again
        if( child && child->cursor().shape() != Qt::ArrowCursor ) return false;

        /*
        check against children from which drag should never be enabled,
        even if mousePress/Move has been passed to the parent
        */
        if( child && (
            qobject_cast<QComboBox*>(child ) ||
            qobject_cast<QProgressBar*>( child ) ||
            qobject_cast<QScrollBar*>( child ) ) )
        { return false; }

        // tool buttons
        if( auto toolButton = qobject_cast<QToolButton*>( widget ) )
        {
            if( dragMode() == StyleConfigData::WD_MINIMAL && !qobject_cast<QToolBar*>(widget->parentWidget() ) ) return false;
            return toolButton->autoRaise() && !toolButton->isEnabled();
        }

        // check menubar
        if( auto menuBar = qobject_cast<QMenuBar*>( widget ) )
        {

            // do not drag from menubars embedded in Mdi windows
            if( findParent<QMdiSubWindow*>( widget ) ) return false;

            // check if there is an active action
            if( menuBar->activeAction() && menuBar->activeAction()->isEnabled() ) return false;

            // check if action at position exists and is enabled
            if( auto action = menuBar->actionAt( position ) )
            {
                if( action->isSeparator() ) return true;
                if( action->isEnabled() ) return false;
            }

            // return true in all other cases
            return true;

        }

        /*
        in MINIMAL mode, anything that has not been already accepted
        and does not come from a toolbar is rejected
        */
        if( dragMode() == StyleConfigData::WD_MINIMAL )
        {
            if( qobject_cast<QToolBar*>( widget ) ) return true;
            else return false;
        }

        /* following checks are relevant only for WD_FULL mode */

        // tabbar. Make sure no tab is under the cursor
        if( auto tabBar = qobject_cast<QTabBar*>( widget ) )
        { return tabBar->tabAt( position ) == -1; }

        /*
        check groupboxes
        prevent drag if unchecking grouboxes
        */
        if( auto groupBox = qobject_cast<QGroupBox*>( widget ) )
        {
            // non checkable group boxes are always ok
            if( !groupBox->isCheckable() ) return true;

            // gather options to retrieve checkbox subcontrol rect
            QStyleOptionGroupBox opt;
            opt.initFrom( groupBox );
            if( groupBox->isFlat() ) opt.features |= QStyleOptionFrame::Flat;
            opt.lineWidth = 1;
            opt.midLineWidth = 0;
            opt.text = groupBox->title();
            opt.textAlignment = groupBox->alignment();
            opt.subControls = (QStyle::SC_GroupBoxFrame | QStyle::SC_GroupBoxCheckBox);
            if (!groupBox->title().isEmpty()) opt.subControls |= QStyle::SC_GroupBoxLabel;

            opt.state |= (groupBox->isChecked() ? QStyle::State_On : QStyle::State_Off);

            // check against groupbox checkbox
            if( groupBox->style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxCheckBox, groupBox ).contains( position ) )
            { return false; }

            // check against groupbox label
            if( !groupBox->title().isEmpty() && groupBox->style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, groupBox ).contains( position ) )
            { return false; }

            return true;

        }

        // labels
        if( auto label = qobject_cast<QLabel*>( widget ) )
        { if( label->textInteractionFlags().testFlag( Qt::TextSelectableByMouse ) ) return false; }

        // abstract item views
        QAbstractItemView* itemView( nullptr );
        if(
            ( itemView = qobject_cast<QListView*>( widget->parentWidget() ) ) ||
            ( itemView = qobject_cast<QTreeView*>( widget->parentWidget() ) ) )
        {
            if( widget == itemView->viewport() )
            {
                // QListView
                if( itemView->frameShape() != QFrame::NoFrame ) return false;
                else if(
                    itemView->selectionMode() != QAbstractItemView::NoSelection &&
                    itemView->selectionMode() != QAbstractItemView::SingleSelection &&
                    itemView->model() && itemView->model()->rowCount() ) return false;
                else if( itemView->model() && itemView->indexAt( position ).isValid() ) return false;
            }

        } else if( ( itemView = qobject_cast<QAbstractItemView*>( widget->parentWidget() ) ) ) {


            if( widget == itemView->viewport() )
            {
                // QAbstractItemView
                if( itemView->frameShape() != QFrame::NoFrame ) return false;
                else if( itemView->indexAt( position ).isValid() ) return false;
            }

        } else if( auto graphicsView =  qobject_cast<QGraphicsView*>( widget->parentWidget() ) )  {

            if( widget == graphicsView->viewport() )
            {
                // QGraphicsView
                if( graphicsView->frameShape() != QFrame::NoFrame ) return false;
                else if( graphicsView->dragMode() != QGraphicsView::NoDrag ) return false;
                else if( graphicsView->itemAt( position ) ) return false;
            }

        }

        return true;

    }

    //____________________________________________________________
    void WindowManager::resetDrag()
    {

        if( (!useWMMoveResize() ) && _target && _cursorOverride ) {

          qApp->restoreOverrideCursor();
          _cursorOverride = false;

        }

        _target.clear();
        #if BREEZE_HAVE_QTQUICK
        _quickTarget.clear();
        #endif
        if( _dragTimer.isActive() ) _dragTimer.stop();
        _dragPoint = QPoint();
        _globalDragPoint = QPoint();
        _dragAboutToStart = false;
        _dragInProgress = false;

    }

    //____________________________________________________________
    void WindowManager::startDrag( QWindow* window, const QPoint& position )
    {

        if( !( enabled() && window ) ) return;
        if( QWidget::mouseGrabber() ) return;

        // ungrab pointer
        if( useWMMoveResize() )
        {

            if( Helper::isX11() ) startDragX11( window, position );
            else if( Helper::isWayland() ) startDragWayland( window, position );

        } else if( !_cursorOverride ) {

            qApp->setOverrideCursor( Qt::SizeAllCursor );
            _cursorOverride = true;

        }

        _dragInProgress = true;

    }

    //_______________________________________________________
    void WindowManager::startDragX11( QWindow* window, const QPoint& position )
    {
        #if BREEZE_HAVE_X11
        // connection
        auto connection( Helper::connection() );

        auto net_connection = connection;
        const qreal dpiRatio = window->devicePixelRatio();
        const QPoint origin = window->screen()->geometry().topLeft();
        const QPoint native = (position - origin) * dpiRatio + origin;

        xcb_ungrab_pointer( connection, XCB_TIME_CURRENT_TIME );
        NETRootInfo( net_connection, NET::WMMoveResize ).moveResizeRequest(
            window->winId(), native.x(), native.y(), NET::Move );

        #else

        Q_UNUSED( window );
        Q_UNUSED( position );

        #endif
    }

    //_______________________________________________________
    void WindowManager::startDragWayland( QWindow* window, const QPoint& )
    {
        #if BREEZE_HAVE_KWAYLAND
        if( !_seat ) {
            return;
        }

        auto shellSurface = KWayland::Client::ShellSurface::fromWindow(window);
        if( !shellSurface )
        {
            // TODO: also check for xdg-shell in future
            return;
        }

        shellSurface->requestMove( _seat, _waylandSerial );
        #else
        Q_UNUSED( window );
        #endif
    }

    //____________________________________________________________
    bool WindowManager::supportWMMoveResize() const
    {

        #if BREEZE_HAVE_KWAYLAND
        if( Helper::isWayland() )
        {
            return true;
        }
        #endif

        #if BREEZE_HAVE_X11
        return Helper::isX11();
        #else
        return false;
        #endif

    }

    //____________________________________________________________
    bool WindowManager::isDockWidgetTitle( const QWidget* widget ) const
    {

        if( !widget ) return false;
        if( auto dockWidget = qobject_cast<const QDockWidget*>( widget->parent() ) )
        {

            return widget == dockWidget->titleBarWidget();

        } else return false;

    }

}
