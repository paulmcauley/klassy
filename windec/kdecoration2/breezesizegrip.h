#ifndef breezesizegrip_h
#define breezesizegrip_h

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

#include "breezedecoration.h"
#include "config-breeze.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include <QPointer>

#if BREEZE_HAVE_X11
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    //* implements size grip for all widgets
    class SizeGrip: public QWidget
    {

        Q_OBJECT

        public:

        //* constructor
        explicit SizeGrip( Decoration* );

        //* constructor
        virtual ~SizeGrip( void );

        protected Q_SLOTS:

        //* update background color
        void updateActiveState( void );

        //* update position
        void updatePosition( void );

        //* embed into parent widget
        void embed( void );

        protected:

        //*@name event handlers
        //@{

        //* paint
        virtual void paintEvent( QPaintEvent* ) override;

        //* mouse press
        virtual void mousePressEvent( QMouseEvent* ) override;

        //@}

        private:

        //* send resize event
        void sendMoveResizeEvent( QPoint );

        //* grip size
        enum {
            Offset = 0,
            GripSize = 14
        };

        //* decoration
        QPointer<Decoration> m_decoration;

        //* move/resize atom
        #if BREEZE_HAVE_X11
        xcb_atom_t m_moveResizeAtom = 0;
        #endif

    };


}

#endif
