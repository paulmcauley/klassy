#ifndef breezewindowmanager_h
#define breezewindowmanager_h

//////////////////////////////////////////////////////////////////////////////
// breezewindowmanager.h
// pass some window mouse press/release/move event actions to window manager
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

#include "breeze.h"

#include <QEvent>

#include <QBasicTimer>
#include <QObject>
#include <QSet>
#include <QString>
#include <QWidget>

#if HAVE_X11
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    class WindowManager: public QObject
    {

        Q_OBJECT

        public:

        //! constructor
        explicit WindowManager( QObject* );

        //! destructor
        virtual ~WindowManager( void )
        {}

        //! initialize
        /*! read relevant options from BreezeStyleConfigData */
        void initialize( void );

        //! register widget
        void registerWidget( QWidget* );

        //! unregister widget
        void unregisterWidget( QWidget* );

        //! event filter [reimplemented]
        virtual bool eventFilter( QObject*, QEvent* );

        protected:

        //! timer event,
        /*! used to start drag if button is pressed for a long enough time */
        void timerEvent( QTimerEvent* );

        //! mouse press event
        bool mousePressEvent( QObject*, QEvent* );

        //! mouse move event
        bool mouseMoveEvent( QObject*, QEvent* );

        //! mouse release event
        bool mouseReleaseEvent( QObject*, QEvent* );

        //!@name configuration
        //@{

        //! enable state
        bool enabled( void ) const
        { return _enabled; }

        //! enable state
        void setEnabled( bool value )
        { _enabled = value; }

        //! returns true if window manager is used for moving
        bool useWMMoveResize( void ) const
        { return supportWMMoveResize() && _useWMMoveResize; }

        //! use window manager for moving, when available
        void setUseWMMoveResize( bool value )
        { _useWMMoveResize = value; }

        //! drag mode
        int dragMode( void ) const
        { return _dragMode; }

        //! drag mode
        void setDragMode( int value )
        { _dragMode = value; }

        //! drag distance (pixels)
        void setDragDistance( int value )
        { _dragDistance = value; }

        //! drag delay (msec)
        void setDragDelay( int value )
        { _dragDelay = value; }

        //! set list of whiteListed widgets
        /*!
        white list is read from options and is used to adjust
        per-app window dragging issues
        */
        void initializeWhiteList();

        //! set list of blackListed widgets
        /*!
        black list is read from options and is used to adjust
        per-app window dragging issues
        */
        void initializeBlackList( void );

        //@}

        //! returns true if widget is dragable
        bool isDragable( QWidget* );

        //! returns true if widget is dragable
        bool isBlackListed( QWidget* );

        //! returns true if widget is dragable
        bool isWhiteListed( QWidget* ) const;

        //! returns true if drag can be started from current widget
        bool canDrag( QWidget* );

        //! returns true if drag can be started from current widget and position
        /*! child at given position is passed as second argument */
        bool canDrag( QWidget*, QWidget*, const QPoint& );

        //! reset drag
        void resetDrag( void );

        //! start drag
        void startDrag( QWidget*, const QPoint& );

        //! returns true if window manager is used for moving
        /*! right now this is true only for X11 */
        bool supportWMMoveResize( void ) const;

        //! utility function
        bool isDockWidgetTitle( const QWidget* ) const;

        //!@name lock
        //@{

        void setLocked( bool value )
        { _locked = value; }

        //! lock
        bool isLocked( void ) const
        { return _locked; }

        //@}

        //! returns first widget matching given class, or 0L if none
        template<typename T> T findParent( const QWidget* ) const;

        private:

        //! enability
        bool _enabled;

        //! use WM moveResize
        bool _useWMMoveResize;

        //! drag mode
        int _dragMode;

        //! drag distance
        /*! this is copied from kwin::geometry */
        int _dragDistance;

        //! drag delay
        /*! this is copied from kwin::geometry */
        int _dragDelay;

        //! wrapper for exception id
        class ExceptionId: public QPair<QString, QString>
        {
            public:

            //! constructor
            explicit ExceptionId( const QString& value )
            {
                const QStringList args( value.split( QChar::fromLatin1( '@' ) ) );
                if( args.isEmpty() ) return;
                second = args[0].trimmed();
                if( args.size()>1 ) first = args[1].trimmed();
            }

            const QString& appName( void ) const
            { return first; }

            const QString& className( void ) const
            { return second; }

        };

        //! exception set
        typedef QSet<ExceptionId> ExceptionSet;

        //! list of white listed special widgets
        /*!
        it is read from options and is used to adjust
        per-app window dragging issues
        */
        ExceptionSet _whiteList;

        //! list of black listed special widgets
        /*!
        it is read from options and is used to adjust
        per-app window dragging issues
        */
        ExceptionSet _blackList;

        //! drag point
        QPoint _dragPoint;
        QPoint _globalDragPoint;

        //! drag timer
        QBasicTimer _dragTimer;

        //! target being dragged
        /*! Weak pointer is used in case the target gets deleted while drag is in progress */
        WeakPointer<QWidget> _target;

        //! true if drag is about to start
        bool _dragAboutToStart;

        //! true if drag is in progress
        bool _dragInProgress;

        //! true if drag is locked
        bool _locked;

        //! cursor override
        /*! used to keep track of application cursor being overridden when dragging in non-WM mode */
        bool _cursorOverride;

        //! provide application-wise event filter
        /*!
        it us used to unlock dragging and make sure event look is properly restored
        after a drag has occurred
        */
        class AppEventFilter: public QObject
        {

            public:

            //! constructor
            explicit AppEventFilter( WindowManager* parent ):
                QObject( parent ),
                _parent( parent )
            {}

            //! event filter
            virtual bool eventFilter( QObject*, QEvent* );

            protected:

            //! application-wise event.
            /*! needed to catch end of XMoveResize events */
            bool appMouseEvent( QObject*, QEvent* );

            private:

            //! parent
            WindowManager* _parent;

        };

        //! application event filter
        AppEventFilter* _appEventFilter;

        #if HAVE_X11
        xcb_atom_t _moveResizeAtom;
        #endif

        bool _isX11;

        //! allow access of all private members to the app event filter
        friend class AppEventFilter;

    };

    //____________________________________________________________________
    template<typename T>
        T WindowManager::findParent( const QWidget* widget ) const
    {

        if( !widget ) return 0L;
        for( QWidget* parent = widget->parentWidget(); parent; parent = parent->parentWidget() )
        { if( T cast = qobject_cast<T>(parent) ) return cast; }

        return 0L;
    }

}

#endif
