#ifndef breezemdiwindowshadow_h
#define breezemdiwindowshadow_h

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

#include <QEvent>
#include <QObject>
#include <QSet>

#include <QWidget>
#include <QPaintEvent>

#include "breezetileset.h"

namespace Breeze
{

    //* frame shadow
    /** this allows the shadow to be painted over the widgets viewport */
    class MdiWindowShadow: public QWidget
    {

        Q_OBJECT

        public:

        //* constructor
        explicit MdiWindowShadow( QWidget* parent, TileSet shadowTiles ):
          QWidget( parent ),
          _widget( 0L ),
          _shadowTiles( shadowTiles )
        {
            setAttribute( Qt::WA_OpaquePaintEvent, false );
            setAttribute( Qt::WA_TransparentForMouseEvents, true );
            setFocusPolicy( Qt::NoFocus );
        }

        //* destructor
        virtual ~MdiWindowShadow( void )
        {}

        //* update geometry
        void updateGeometry( void );

        //* update ZOrder
        void updateZOrder( void );

        //* set associated window
        void setWidget( QWidget* value )
        { _widget = value; }

        //* associated window
        QWidget* widget( void ) const
        { return _widget; }

        protected:

        //* painting
        virtual void paintEvent(QPaintEvent *);

        private:

        //* associated widget
        QWidget* _widget;

        //* tileset rect, used for painting
        QRect _shadowTilesRect;

        //* tileset used to draw shadow
        TileSet _shadowTiles;

    };

    //* shadow manager
    class MdiWindowShadowFactory: public QObject
    {

        Q_OBJECT

        public:

        //* constructor
        explicit MdiWindowShadowFactory( QObject* );

        //* destructor
        virtual ~MdiWindowShadowFactory( void )
        {}

        //* set shadow tiles
        void setShadowTiles( const TileSet& shadowTiles )
        { _shadowTiles = shadowTiles; }

        //* register widget
        bool registerWidget( QWidget* );

        //* unregister
        void unregisterWidget( QWidget* );

        //* true if widget is registered
        bool isRegistered( const QObject* widget ) const
        { return _registeredWidgets.contains( widget ); }

        //* event filter
        virtual bool eventFilter( QObject*, QEvent*);

        protected:

        //* find shadow matching a given object
        MdiWindowShadow* findShadow( QObject* ) const;

        //* install shadows on given widget
        void installShadow( QObject* );

        //* remove shadows from widget
        void removeShadow( QObject* );

        //* hide shadows
        void hideShadows( QObject* object ) const
        {
            if( MdiWindowShadow* windowShadow = findShadow( object ) )
            { windowShadow->hide(); }
        }

        //* update ZOrder
        void updateShadowZOrder( QObject* object ) const
        {
            if( MdiWindowShadow* windowShadow = findShadow( object ) )
            {
                if( !windowShadow->isVisible() ) windowShadow->show();
                windowShadow->updateZOrder();
            }
        }

        //* update shadows geometry
        void updateShadowGeometry( QObject* object ) const
        {
            if( MdiWindowShadow* windowShadow = findShadow( object ) )
            { windowShadow->updateGeometry(); }
        }

        //* update shadows
        void update( QObject* object ) const
        {
            if( MdiWindowShadow* windowShadow = findShadow( object ) )
            { windowShadow->update(); }
        }

        protected Q_SLOTS:

        //* triggered by object destruction
        void widgetDestroyed( QObject* );

        private:

        //* set of registered widgets
        QSet<const QObject*> _registeredWidgets;

        //* tileset used to draw shadow
        TileSet _shadowTiles;

    };

}

#endif
