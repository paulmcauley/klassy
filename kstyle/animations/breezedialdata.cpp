//////////////////////////////////////////////////////////////////////////////
// breezedialdata.cpp
// data container for QTabBar animations
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

#include "breezedialdata.h"
#include "breezedialdata.moc"

#include <QHoverEvent>
#include <QDial>

namespace Breeze
{

    //______________________________________________
    DialData::DialData( QObject* parent, QWidget* target, int duration ):
        SliderData( parent, target, duration ),
        _position( -1, -1 )
    { target->installEventFilter( this ); }

    //______________________________________________
    bool DialData::eventFilter( QObject* object, QEvent* event )
    {

        if( object != target().data() )
        { return SliderData::eventFilter( object, event ); }

        // check event type
        switch( event->type() )
        {

            case QEvent::HoverEnter:
            case QEvent::HoverMove:
            hoverMoveEvent( object, event );
            break;

            case QEvent::HoverLeave:
            hoverLeaveEvent( object, event );
            break;

            default: break;

        }

        return SliderData::eventFilter( object, event );

    }

    //______________________________________________
    void DialData::hoverMoveEvent(  QObject* object, QEvent* event )
    {

        // try cast object to dial
        QDial* scrollBar( qobject_cast<QDial*>( object ) );
        if( !scrollBar || scrollBar->isSliderDown() ) return;

        // cast event
        QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);

        // store position
        _position = hoverEvent->pos();

        // trigger animation if position match handle rect
        updateState( _handleRect.contains( _position ) );

    }


    //______________________________________________
    void DialData::hoverLeaveEvent(  QObject*, QEvent* )
    {

        // reset hover state
        updateState( false );

        // reset mouse position
        _position = QPoint( -1, -1 );
    }

}
