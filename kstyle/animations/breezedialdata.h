#ifndef breezedial_data_h
#define breezedial_data_h

//////////////////////////////////////////////////////////////////////////////
// breezedialdata.h
// data container for QDial animations
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "breezesliderdata.h"

namespace Breeze
{

    //! dial data
    class DialData: public SliderData
    {

        Q_OBJECT

        public:

        //! constructor
        DialData( QObject* parent, QWidget* target, int );

        //! destructor
        virtual ~DialData( void )
        {}

        //! event filter
        virtual bool eventFilter( QObject*, QEvent* );

        //! subcontrol rect
        virtual void setHandleRect( const QRect& rect )
        { _handleRect = rect; }

        //! mouse position
        QPoint position( void ) const
        { return _position; }

        protected:

        //! hoverMoveEvent
        virtual void hoverMoveEvent( QObject*, QEvent* );

        //! hoverMoveEvent
        virtual void hoverLeaveEvent( QObject*, QEvent* );

        private:

        //! rect
        QRect _handleRect;

        //! mouse position
        QPoint _position;

    };

}

#endif
