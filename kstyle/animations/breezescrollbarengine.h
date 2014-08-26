#ifndef breezescrollbarengine_h
#define breezescrollbarengine_h

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
#include "breezescrollbardata.h"

namespace Breeze
{

    //* stores scrollbar hovered action and timeLine
    class ScrollBarEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit ScrollBarEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~ScrollBarEngine( void )
        {}

        //* register scrollbar
        virtual bool registerWidget( QWidget* );

        //* true if widget is animated
        virtual bool isAnimated( const QObject* object, QStyle::SubControl control );

        //* animation opacity
        virtual qreal opacity( const QObject* object, QStyle::SubControl control )
        { return isAnimated( object, control ) ? _data.find( object ).data()->opacity( control ):AnimationData::OpacityInvalid; }

        //* return true if given subcontrol is hovered
        virtual bool isHovered( const QObject* object, QStyle::SubControl control )
        {
            if( DataMap<ScrollBarData>::Value data = _data.find( object ) ) return data.data()->isHovered( control );
            else return false;
        }

        //* control rect associated to object
        virtual QRect subControlRect( const QObject* object, QStyle::SubControl control )
        {
            if( DataMap<ScrollBarData>::Value data = _data.find( object ) ) return data.data()->subControlRect( control );
            else return QRect();
        }

        //* control rect
        virtual void setSubControlRect( const QObject* object, QStyle::SubControl control, const QRect& rect )
        {
            if( DataMap<ScrollBarData>::Value data = _data.find( object ) )
            { data.data()->setSubControlRect( control, rect ); }
        }

        //* control rect
        virtual void updateState( const QObject* object, bool state )
        {
            if( DataMap<ScrollBarData>::Value data = _data.find( object ) )
            { data.data()->updateState( state ); }
        }

        //* mouse position
        virtual QPoint position( const QObject* object )
        {
            if( DataMap<ScrollBarData>::Value data = _data.find( object ) ) return data.data()->position();
            else return QPoint( -1, -1 );
        }

        //* enability
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            /*
            do not disable the map directly, because the contained BreezeScrollbarData
            are also used in non animated mode to store scrollbar arrows rect. However
            do disable all contains DATA object, in order to prevent actual animations
            */
            foreach( const DataMap<ScrollBarData>::Value data, _data )
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
        DataMap<ScrollBarData> _data;

    };

}

#endif
