#ifndef breezewindowmanager_h
#define breezewindowmanager_h

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

#include "breeze.h"
#include "breezestyleconfigdata.h"
#include "config-breeze.h"

#include <QEvent>

#include <QApplication>
#include <QBasicTimer>
#include <QObject>
#include <QSet>
#include <QString>
#include <QWidget>

#if BREEZE_HAVE_QTQUICK
#include <QQuickItem>
#endif

#if BREEZE_HAVE_KWAYLAND
namespace KWayland
{
    namespace Client
    {
        class Pointer;
        class Seat;
    }
}
#endif

namespace Breeze
{

    class WindowManager: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        explicit WindowManager( QObject* );

        //* initialize
        /** read relevant options from config */
        void initialize();

        //* register widget
        void registerWidget( QWidget* );

        #if BREEZE_HAVE_QTQUICK
        //* register quick item
        void registerQuickItem( QQuickItem* );
        #endif

        //* unregister widget
        void unregisterWidget( QWidget* );

        //* event filter [reimplemented]
        bool eventFilter( QObject*, QEvent* ) override;

        protected:

        //* timer event,
        /** used to start drag if button is pressed for a long enough time */
        void timerEvent( QTimerEvent* ) override;

        //* mouse press event
        bool mousePressEvent( QObject*, QEvent* );

        //* mouse move event
        bool mouseMoveEvent( QObject*, QEvent* );

        //* mouse release event
        bool mouseReleaseEvent( QObject*, QEvent* );

        //*@name configuration
        //@{

        //* enable state
        bool enabled() const
        { return _enabled; }

        //* enable state
        void setEnabled( bool value )
        { _enabled = value; }

        //* returns true if window manager is used for moving
        bool useWMMoveResize() const
        { return supportWMMoveResize() && _useWMMoveResize; }

        //* use window manager for moving, when available
        void setUseWMMoveResize( bool value )
        { _useWMMoveResize = value; }

        //* drag mode
        int dragMode() const
        { return _dragMode; }

        //* drag mode
        void setDragMode( int value )
        { _dragMode = value; }

        //* drag distance (pixels)
        void setDragDistance( int value )
        { _dragDistance = value; }

        //* drag delay (msec)
        void setDragDelay( int value )
        { _dragDelay = value; }

        //* set list of whiteListed widgets
        /**
        white list is read from options and is used to adjust
        per-app window dragging issues
        */
        void initializeWhiteList();

        //* set list of blackListed widgets
        /**
        black list is read from options and is used to adjust
        per-app window dragging issues
        */
        void initializeBlackList();

        //* initializes the Wayland specific parts
        void initializeWayland();

        //* The Wayland Seat's hasPointer property changed
        void waylandHasPointerChanged(bool hasPointer);

        //@}

        //* returns true if widget is dragable
        bool isDragable( QWidget* );

        //* returns true if widget is dragable
        bool isBlackListed( QWidget* );

        //* returns true if widget is dragable
        bool isWhiteListed( QWidget* ) const;

        //* returns true if drag can be started from current widget
        bool canDrag( QWidget* );

        //* returns true if drag can be started from current widget and position
        /** child at given position is passed as second argument */
        bool canDrag( QWidget*, QWidget*, const QPoint& );

        //* reset drag
        void resetDrag();

        //* start drag
        void startDrag( QWindow*, const QPoint& );

        //* X11 specific implementation for startDrag
        void startDragX11( QWindow*, const QPoint& );

        //* Wayland specific implementation for startDrag
        void startDragWayland( QWindow*, const QPoint& );

        //* returns true if window manager is used for moving
        /** right now this is true only for X11 */
        bool supportWMMoveResize() const;

        //* utility function
        bool isDockWidgetTitle( const QWidget* ) const;

        //*@name lock
        //@{

        void setLocked( bool value )
        { _locked = value; }

        //* lock
        bool isLocked() const
        { return _locked; }

        //@}

        //* returns first widget matching given class, or nullptr if none
        template<typename T> T findParent( const QWidget* ) const;

        private:

        //* enability
        bool _enabled = true;

        //* use WM moveResize
        bool _useWMMoveResize = true;

        //* drag mode
        int _dragMode = StyleConfigData::WD_FULL;

        //* drag distance
        /** this is copied from kwin::geometry */
        int _dragDistance = QApplication::startDragDistance();

        //* drag delay
        /** this is copied from kwin::geometry */
        int _dragDelay = QApplication::startDragTime();

        //* wrapper for exception id
        class ExceptionId
        {
            public:

            //* constructor
            explicit ExceptionId( const QString& value )
            {
                const QStringList args( value.split( QChar::fromLatin1( '@' ) ) );
                if( args.isEmpty() ) return;
                _exception.second = args[0].trimmed();
                if( args.size()>1 ) _exception.first = args[1].trimmed();
            }

            const QString& appName() const
            { return _exception.first; }

            const QString& className() const
            { return _exception.second; }

            private:

            QPair<QString, QString> _exception;

            friend uint qHash( const ExceptionId& value )
            { return qHash(value._exception); }

            friend bool operator == ( const ExceptionId& lhs, const ExceptionId& rhs )
            { return lhs._exception == rhs._exception; }

        };

        //* exception set
        using ExceptionSet = QSet<ExceptionId>;

        //* list of white listed special widgets
        /**
        it is read from options and is used to adjust
        per-app window dragging issues
        */
        ExceptionSet _whiteList;

        //* list of black listed special widgets
        /**
        it is read from options and is used to adjust
        per-app window dragging issues
        */
        ExceptionSet _blackList;

        //* drag point
        QPoint _dragPoint;
        QPoint _globalDragPoint;

        //* drag timer
        QBasicTimer _dragTimer;

        //* target being dragged
        /** Weak pointer is used in case the target gets deleted while drag is in progress */
        WeakPointer<QWidget> _target;

        #if BREEZE_HAVE_QTQUICK
        WeakPointer<QQuickItem> _quickTarget;
        #endif

        //* true if drag is about to start
        bool _dragAboutToStart = false;

        //* true if drag is in progress
        bool _dragInProgress = false;

        //* true if drag is locked
        bool _locked = false;

        //* cursor override
        /** used to keep track of application cursor being overridden when dragging in non-WM mode */
        bool _cursorOverride = false;

        //* application event filter
        QObject* _appEventFilter = nullptr;

        #if BREEZE_HAVE_KWAYLAND

        //* The Wayland seat object which needs to be passed to move requests.
        KWayland::Client::Seat* _seat = nullptr;

        //* The Wayland pointer object where we get pointer events on.
        KWayland::Client::Pointer* _pointer = nullptr;

        //* latest serial which needs to be passed to the move requests.
        quint32 _waylandSerial = 0;
        #endif

        //* allow access of all private members to the app event filter
        friend class AppEventFilter;

    };

    //____________________________________________________________________
    template<typename T>
        T WindowManager::findParent( const QWidget* widget ) const
    {

        if( !widget ) return nullptr;
        for( QWidget* parent = widget->parentWidget(); parent; parent = parent->parentWidget() )
        { if( T cast = qobject_cast<T>(parent) ) return cast; }

        return nullptr;
    }

}

#endif
