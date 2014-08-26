#ifndef breezeslider_data_h
#define breezeslider_data_h

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

#include "breezegenericdata.h"

#include <QStyle>

namespace Breeze
{

    //* scrollbar data
    class SliderData: public GenericData
    {

        Q_OBJECT

        public:

        //* constructor
        SliderData( QObject* parent, QWidget* target, int duration ):
            GenericData( parent, target, duration ),
            _handleHovered( false )
        {}

        //* destructor
        virtual ~SliderData( void )
        {}

        //* update state
        virtual bool updateState( bool );

        protected:

        //* hover
        virtual bool handleHovered( void ) const
        { return _handleHovered; }

        //* hover
        virtual void setHandleHovered( bool value )
        { _handleHovered = value; }

        //* update slider
        virtual void updateSlider( QStyle::SubControl );

        private:

        //* true when slider is hovered
        bool _handleHovered;

    };

}

#endif
