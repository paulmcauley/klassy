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

#include "breezesliderdata.h"
#include "breezesliderdata.moc"

#include <QHoverEvent>
#include <QSlider>
#include <QStyleOptionSlider>

Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider*);


namespace Breeze
{

    //______________________________________________
    bool SliderData::updateState( bool state )
    {
        if( state == _handleHovered ) return false;
        updateSlider( state ? QStyle::SC_SliderHandle : QStyle::SC_None );
        return true;
    }

    //_____________________________________________________________________
    void SliderData::updateSlider( QStyle::SubControl hoverControl )
    {

        if( hoverControl == QStyle::SC_SliderHandle )
        {

            if( !handleHovered() ) {
                setHandleHovered( true );
                if( enabled() )
                {
                    animation().data()->setDirection( Animation::Forward );
                    if( !animation().data()->isRunning() ) animation().data()->start();
                } else setDirty();
            }

        } else {

            if( handleHovered() )
            {
                setHandleHovered( false );
                if( enabled() )
                {
                    animation().data()->setDirection( Animation::Backward );
                    if( !animation().data()->isRunning() ) animation().data()->start();
                } else setDirty();
            }

        }
    }

}
