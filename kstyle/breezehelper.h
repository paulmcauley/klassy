#ifndef breeze_helper_h
#define breeze_helper_h

/*
 * Copyright 2009-2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * Copyright 2008 Long Huynh Huu <long.upcase@googlemail.com>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright 2007 Fredrik H?glund <fredrik@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QScopedPointer>

#if HAVE_X11
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    //! breeze style helper class.
    /*! contains utility functions used at multiple places in both breeze style and breeze window decoration */
    class Helper
    {
        public:

        //! constructor
        explicit Helper()
        {}

        //! destructor
        virtual ~Helper()
        {}

        template <typename T> class ScopedPointer: public QScopedPointer<T, QScopedPointerPodDeleter>
        {
            public:

            //! constructor
            ScopedPointer( T* t ):
                QScopedPointer<T, QScopedPointerPodDeleter>( t )
            {}

            //! destructor
            virtual ~ScopedPointer( void )
            {}

        };

    };

}

#endif
