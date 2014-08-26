#ifndef breezeframeshadow_h
#define breezeframeshadow_h

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
#include "breezeaddeventfilter.h"
#include "breezehelper.h"

#include <QEvent>
#include <QObject>
#include <QSet>

#include <QWidget>
#include <QPaintEvent>
#include <KColorScheme>

namespace Breeze
{

    //* shadow manager
    class FrameShadowFactory: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        explicit FrameShadowFactory( QObject* parent ):
        QObject( parent )
        {}

        //* destructor
        virtual ~FrameShadowFactory( void )
        {}

        //* register widget
        bool registerWidget( QWidget*, Helper& );

        //* unregister
        void unregisterWidget( QWidget* );

        //* true if widget is registered
        bool isRegistered( const QWidget* widget ) const
        { return _registeredWidgets.contains( widget ); }

        //* event filter
        virtual bool eventFilter( QObject*, QEvent*);

        //* update state
        void updateState( const QWidget*, bool focus, bool hover, qreal opacity, AnimationMode ) const;

        protected:

        //* install shadows on given widget
        void installShadows( QWidget*, Helper& );

        //* remove shadows from widget
        void removeShadows( QWidget* );

        //* update shadows geometry
        void updateShadowsGeometry( QObject* ) const;

        //* raise shadows
        void raiseShadows( QObject* ) const;

        //* update shadows
        void update( QObject* ) const;

        //* install shadow on given side
        void installShadow( QWidget*, Helper&, ShadowArea ) const;

        protected Q_SLOTS:

        //* triggered by object destruction
        void widgetDestroyed( QObject* );

        private:

        //* needed to block ChildAdded events when creating shadows
        AddEventFilter _addEventFilter;

        //* set of registered widgets
        QSet<const QObject*> _registeredWidgets;

    };

    //* frame shadow
    /** this allows the shadow to be painted over the widgets viewport */
    class FrameShadowBase: public QWidget
    {

        Q_OBJECT

        public:

        //* constructor
        explicit FrameShadowBase( ShadowArea area ):
            _area( area )
        {}

        //* destructor
        virtual ~FrameShadowBase( void )
        {}

        //* shadow area
        void setShadowArea(ShadowArea area)
        { _area = area; }

        //* shadow area
        const ShadowArea& shadowArea() const
        { return _area; }

        //* update geometry
        virtual void updateGeometry( void ) = 0;

        //* update state
        virtual void updateState( bool, bool, qreal, AnimationMode )
        {}

        protected:

        //* event handler
        virtual bool event(QEvent *e);

        //* initialization
        virtual void init();

        //* return viewport associated to parent widget
        virtual QWidget* viewport( void ) const;

        private:

        //* shadow area
        ShadowArea _area;

    };

    //* frame shadow
    /** this allows the shadow to be painted over the widgets viewport */
    class SunkenFrameShadow : public FrameShadowBase
    {
        Q_OBJECT

        public:

        //* constructor
        SunkenFrameShadow( ShadowArea area, Helper& helper ):
            FrameShadowBase( area ),
            _helper( helper ),
            _hasFocus( false ),
            _mouseOver( false ),
            _opacity( -1 ),
            _mode( AnimationNone )
        { init(); }


        //* destructor
        virtual ~SunkenFrameShadow()
        {}

        //* update geometry
        virtual void updateGeometry( void );

        //* update state
        void updateState( bool focus, bool hover, qreal opacity, AnimationMode );

        protected:

        //* painting
        virtual void paintEvent(QPaintEvent *);

        private:

        //* shadow sizes
        enum
        {
            ShadowSizeTop = 3,
            ShadowSizeBottom = 3,
            ShadowSizeLeft = 3,
            ShadowSizeRight = 3
        };

        //* helper
        Helper& _helper;

        //*@name widget state
        //@{
        bool _hasFocus;
        bool _mouseOver;
        qreal _opacity;
        AnimationMode _mode;
    };
}

#endif
