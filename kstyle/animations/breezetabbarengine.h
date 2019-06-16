#ifndef breezetabbarengine_h
#define breezetabbarengine_h

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
#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezetabbardata.h"

namespace Breeze
{

    //* stores tabbar hovered action and timeLine
    class TabBarEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit TabBarEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* register tabbar
        bool registerWidget( QWidget* );

        //* true if widget hover state is changed
        bool updateState( const QObject*, const QPoint&, AnimationMode, bool );

        //* true if widget is animated
        bool isAnimated( const QObject* object, const QPoint& point, AnimationMode );

        //* animation opacity
        qreal opacity( const QObject* object, const QPoint& point, AnimationMode mode )
        { return isAnimated( object, point, mode ) ? data( object, mode ).data()->opacity( point ) : AnimationData::OpacityInvalid; }

        //* enability
        void setEnabled( bool value ) override
        {
            BaseEngine::setEnabled( value );
            _hoverData.setEnabled( value );
            _focusData.setEnabled( value );
        }

        //* duration
        void setDuration( int value ) override
        {
            BaseEngine::setDuration( value );
            _hoverData.setDuration( value );
            _focusData.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        bool unregisterWidget( QObject* object ) override
        {
            if( !object ) return false;
            bool found = false;
            if( _hoverData.unregisterWidget( object ) ) found = true;
            if( _focusData.unregisterWidget( object ) ) found = true;
            return found;
        }

        private:

        //* returns data associated to widget
        DataMap<TabBarData>::Value data( const QObject*, AnimationMode );

        //* data map
        DataMap<TabBarData> _hoverData;
        DataMap<TabBarData> _focusData;

    };

}

#endif
