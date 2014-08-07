//////////////////////////////////////////////////////////////////////////////
// breezetabbarengine.cpp
// stores event filters and maps widgets to timelines for animations
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

#include "breezetabbarengine.h"
#include "breezetabbarengine.moc"

#include <QEvent>

namespace Breeze
{

    //____________________________________________________________
    bool TabBarEngine::registerWidget( QWidget* widget )
    {

        if( !widget ) return false;

        // create new data class
        if( !_hoverData.contains( widget ) ) _hoverData.insert( widget, new TabBarData( this, widget, duration() ), enabled() );
        if( !_focusData.contains( widget ) ) _focusData.insert( widget, new TabBarData( this, widget, duration() ), enabled() );

        // connect destruction signal
        connect( widget, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterWidget(QObject*)), Qt::UniqueConnection );
        return true;

    }

    //____________________________________________________________
    bool TabBarEngine::updateState( const QObject* object, const QPoint& position, AnimationMode mode, bool value )
    {
        DataMap<TabBarData>::Value data( TabBarEngine::data( object, mode ) );
        return ( data && data.data()->updateState( position, value ) );
    }

    //____________________________________________________________
    bool TabBarEngine::isAnimated( const QObject* object, const QPoint& position, AnimationMode mode )
    {

        DataMap<TabBarData>::Value data( TabBarEngine::data( object, mode ) );
        return ( data && data.data()->animation( position ) && data.data()->animation( position ).data()->isRunning() );

    }

    //____________________________________________________________
    DataMap<TabBarData>::Value TabBarEngine::data( const QObject* object, AnimationMode mode )
    {

        switch( mode )
        {
            case AnimationHover: return _hoverData.find( object ).data();
            case AnimationFocus: return _focusData.find( object ).data();
            default: return DataMap<TabBarData>::Value();
        }

    }

}
