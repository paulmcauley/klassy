#ifndef breezeaddeventfilter_h
#define breezeaddeventfilter_h

//////////////////////////////////////////////////////////////////////////////
// breezeaddeventfilter.h
// used to block add a child to a widget, blocking AddChild parent events
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Largely inspired from BeSpin style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QEvent>

namespace Breeze
{

    class AddEventFilter: public QObject
    {

        public:

        //* constructor
        AddEventFilter( void ):
            QObject()
            {}

        //* destructor
        virtual ~AddEventFilter( void )
        {}


        //* event filter
        /** blocks all AddChild events */
        virtual bool eventFilter( QObject*, QEvent* event )
        { return event->type() == QEvent::ChildAdded; }

    };

}

#endif
