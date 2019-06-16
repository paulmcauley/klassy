#ifndef breezebusyindicatorengine_h
#define breezebusyindicatorengine_h

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

#include "breezeanimation.h"
#include "breezebaseengine.h"
#include "breezebusyindicatordata.h"
#include "breezedatamap.h"

namespace Breeze
{

    //* handles progress bar animations
    class BusyIndicatorEngine: public BaseEngine
    {

        Q_OBJECT

        //* declare opacity property
        Q_PROPERTY( int value READ value WRITE setValue )

        public:

        //* constructor
        explicit BusyIndicatorEngine( QObject* );

        //*@name accessors
        //@{

        //* true if widget is animated
        bool isAnimated( const QObject* );

        //* value
        int value() const
        { return _value; }

        //@}

        //*@name modifiers
        //@{

        //* register progressbar
        bool registerWidget( QObject* );

        //* duration
        void setDuration( int ) override;

        //* set object as animated
        void setAnimated( const QObject*, bool );

        //* opacity
        void setValue( int value );

        //@}

        public Q_SLOTS:

        //* remove widget from map
        bool unregisterWidget( QObject* ) override;

        protected:

        //* returns data associated to widget
        DataMap<BusyIndicatorData>::Value data( const QObject* );

        private:

        //* map widgets to progressbar data
        DataMap<BusyIndicatorData> _data;

        //* animation
        Animation::Pointer _animation;

        //* value
        int _value = 0;

    };

}

#endif
