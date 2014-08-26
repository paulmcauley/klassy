#ifndef breezesliderengine_h
#define breezesliderengine_h

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
#include "breezesliderdata.h"

namespace Breeze
{

    //* stores slider hovered action and timeLine
    class SliderEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit SliderEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~SliderEngine( void )
        {}

        //* register slider
        virtual bool registerWidget( QWidget* );

        //* true if widget is animated
        virtual bool isAnimated( const QObject* object )
        {
            if( DataMap<SliderData>::Value data = _data.find( object ) )
            {

                return data.data()->animation().data()->isRunning();

            } else return false;

        }

        //* update state
        virtual bool updateState( const QObject* object, bool state )
        {

            if( DataMap<SliderData>::Value data = _data.find( object ) )
            {

                return data.data()->updateState( state );

            } else return false;

        }

        //* animation opacity
        virtual qreal opacity( const QObject* object )
        { return isAnimated( object ) ? _data.find( object ).data()->opacity() : AnimationData::OpacityInvalid; }

        //* enability
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            _data.setEnabled( value );
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
        DataMap<SliderData> _data;

    };

}

#endif
