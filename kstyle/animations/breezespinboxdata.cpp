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

#include "breezespinboxdata.h"

namespace Breeze
{

    //________________________________________________
    SpinBoxData::SpinBoxData( QObject* parent, QWidget* target, int duration ):
        AnimationData( parent, target )
    {
        _upArrowData._animation = new Animation( duration, this );
        _downArrowData._animation = new Animation( duration, this );
        setupAnimation( upArrowAnimation(), "upArrowOpacity" );
        setupAnimation( downArrowAnimation(), "downArrowOpacity" );
    }

    //______________________________________________
    bool SpinBoxData::Data::updateState( bool value )
    {
        if( _state == value ) return false;
        else {

            _state = value;
            _animation.data()->setDirection( _state ? Animation::Forward : Animation::Backward );
            if( !_animation.data()->isRunning() ) _animation.data()->start();
            return true;

        }
    }

}
