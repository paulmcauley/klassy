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

        //* register widget
        bool registerWidget( QWidget*, Helper& );

        //* unregister
        void unregisterWidget( QWidget* );

        //* true if widget is registered
        bool isRegistered( const QWidget* widget ) const
        { return _registeredWidgets.contains( widget ); }

        //* event filter
        bool eventFilter( QObject*, QEvent*) override;

        //* update state
        void updateState( const QWidget*, bool focus, bool hover, qreal opacity, AnimationMode ) const;

        //* update shadows geometry
        void updateShadowsGeometry( const QObject*, QRect ) const;

        protected:

        //* install shadows on given widget
        void installShadows( QWidget*, Helper& );

        //* remove shadows from widget
        void removeShadows( QWidget* );

        //* raise shadows
        void raiseShadows( QObject* ) const;

        //* update shadows
        void update( QObject* ) const;

        //* install shadow on given side
        void installShadow( QWidget*, Helper&, Side ) const;

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
    class FrameShadow : public QWidget
    {
        Q_OBJECT

        public:

        //* constructor
        FrameShadow( Side, Helper& );

        //* update geometry
        virtual void updateGeometry( QRect );

        //* update state
        void updateState( bool focus, bool hover, qreal opacity, AnimationMode );

        protected:

        //* painting
        void paintEvent( QPaintEvent* ) override;

        //* return viewport associated to parent widget
        QWidget* viewport() const;

        private:

        //* helper
        Helper& _helper;

        //* shadow area
        Side _area;

        //* margins
        /** offsets between update rect and parent widget rect. It is set via updateGeometry */
        QMargins _margins;

        //*@name widget state
        //@{
        bool _hasFocus = false;
        bool _mouseOver = false;
        qreal _opacity = -1;
        AnimationMode _mode = AnimationNone;
        //@}

    };
}

#endif
