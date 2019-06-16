#ifndef breezetabbar_data_h
#define breezetabbar_data_h

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

#include "breezeanimationdata.h"

#include <QTabBar>

namespace Breeze
{

    //* tabbars
    class TabBarData: public AnimationData
    {

        Q_OBJECT

        //* declare opacity property
        Q_PROPERTY( qreal currentOpacity READ currentOpacity WRITE setCurrentOpacity )
        Q_PROPERTY( qreal previousOpacity READ previousOpacity WRITE setPreviousOpacity )

        public:

        //* constructor
        TabBarData( QObject* parent, QWidget* target, int duration );

        //* duration
        void setDuration( int duration ) override
        {
            currentIndexAnimation().data()->setDuration( duration );
            previousIndexAnimation().data()->setDuration( duration );
        }

        //* update state
        bool updateState( const QPoint&, bool );

        //*@name current index handling
        //@{

        //* current opacity
        qreal currentOpacity() const
        { return _current._opacity; }

        //* current opacity
        void setCurrentOpacity( qreal value )
        {
            if( _current._opacity == value ) return;
            _current._opacity = value;
            setDirty();
        }

        //* current index
        int currentIndex() const
        { return _current._index; }

        //* current index
        void setCurrentIndex( int index )
        { _current._index = index; }

        //* current index animation
        const Animation::Pointer& currentIndexAnimation() const
        { return _current._animation; }

        //@}

        //*@name previous index handling
        //@{

        //* previous opacity
        qreal previousOpacity() const
        { return _previous._opacity; }

        //* previous opacity
        void setPreviousOpacity( qreal value )
        {
            if( _previous._opacity == value ) return;
            _previous._opacity = value;
            setDirty();
        }

        //* previous index
        int previousIndex() const
        { return _previous._index; }

        //* previous index
        void setPreviousIndex( int index )
        { _previous._index = index; }

        //* previous index Animation
        const Animation::Pointer& previousIndexAnimation() const
        { return _previous._animation; }

        //@}

        //* return Animation associated to action at given position, if any
        Animation::Pointer animation( const QPoint& position ) const;

        //* return opacity associated to action at given position, if any
        qreal opacity( const QPoint& position ) const;

        private:

        //* container for needed animation data
        class Data
        {
            public:

            //* default constructor
            Data():
                _opacity(0),
                _index(-1)
            {}

            Animation::Pointer _animation;
            qreal _opacity;
            int _index;
        };

        //* current tab animation data (for hover enter animations)
        Data _current;

        //* previous tab animations data (for hover leave animations)
        Data _previous;

    };

}

#endif
