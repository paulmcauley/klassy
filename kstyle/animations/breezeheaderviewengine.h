#ifndef breezeheaderviewengine_h
#define breezeheaderviewengine_h

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
#include "breezeheaderviewdata.h"

namespace Breeze
{

    //* stores headerview hovered action and timeLine
    class HeaderViewEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit HeaderViewEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* register headerview
        bool registerWidget( QWidget* );

        //* true if widget hover state is changed
        bool updateState( const QObject*, const QPoint&, bool );

        //* true if widget is animated
        bool isAnimated( const QObject* object, const QPoint& point )
        {
            if( DataMap<HeaderViewData>::Value data = _data.find( object ) )
            { if( Animation::Pointer animation = data.data()->animation( point ) ) return animation.data()->isRunning(); }
            return false;
        }

        //* animation opacity
        qreal opacity( const QObject* object, const QPoint& point )
        { return isAnimated( object, point ) ? _data.find( object ).data()->opacity( point ) : AnimationData::OpacityInvalid; }

        //* enability
        void setEnabled( bool value ) override
        {
            BaseEngine::setEnabled( value );
            _data.setEnabled( value );
        }

        //* duration
        void setDuration( int value ) override
        {
            BaseEngine::setDuration( value );
            _data.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        bool unregisterWidget( QObject* object ) override
        { return _data.unregisterWidget( object ); }

        private:

        //* data map
        DataMap<HeaderViewData> _data;

    };

}

#endif
