#ifndef breezedialengine_h
#define breezedialengine_h

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

#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezedialdata.h"

namespace Breeze
{

    //* stores dial hovered action and timeLine
    class DialEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit DialEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~DialEngine( void )
        {}

        //* register dial
        virtual bool registerWidget( QWidget* );

        //* true if widget is animated
        virtual bool isAnimated( const QObject* object )
        {
            if( DataMap<DialData>::Value data = _data.find( object ) )
            {

                return data.data()->animation().data()->isRunning();

            } else return false;

        }

        //* update state
        virtual bool updateState( const QObject* object, bool state )
        {

            if( DataMap<DialData>::Value data = _data.find( object ) )
            {

                return data.data()->updateState( state );

            } else return false;

        }

        //* animation opacity
        virtual qreal opacity( const QObject* object )
        { return isAnimated( object ) ? _data.find( object ).data()->opacity() : AnimationData::OpacityInvalid; }

        //* control rect
        virtual void setHandleRect( const QObject* object, const QRect& rect )
        {
            if( DataMap<DialData>::Value data = _data.find( object ) )
            { data.data()->setHandleRect( rect ); }
        }

        //* mouse position
        virtual QPoint position( const QObject* object )
        {
            if( DataMap<DialData>::Value data = _data.find( object ) ) return data.data()->position();
            else return QPoint( -1, -1 );
        }

        //* enability
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            /*
            do not disable the map directly, because the contained BreezeScrollbarData
            are also used in non animated mode to store dial handle rect. However
            do disable all contains DATA object, in order to prevent actual animations
            */
            foreach( const DataMap<DialData>::Value data, _data )
            { if( data ) data.data()->setEnabled( value ); }

        }

        //* duration
        virtual void setDuration( int value )
        {
            BaseEngine::setDuration( value );
            _data.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        virtual bool unregisterWidget( QObject* object )
        { return _data.unregisterWidget( object ); }

        private:

        //* data map
        DataMap<DialData> _data;

    };

}

#endif
