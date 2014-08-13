
//////////////////////////////////////////////////////////////////////////////
// breezewidgetstateengine.h
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

#include "breezewidgetstateengine.h"
#include "breezewidgetstateengine.moc"

#include "breezeenabledata.h"

namespace Breeze
{

    //____________________________________________________________
    bool WidgetStateEngine::registerWidget( QWidget* widget, AnimationModes mode )
    {

        if( !widget ) return false;
        if( mode&AnimationHover && !_hoverData.contains( widget ) ) { _hoverData.insert( widget, new WidgetStateData( this, widget, duration() ), enabled() ); }
        if( mode&AnimationFocus && !_focusData.contains( widget ) ) { _focusData.insert( widget, new WidgetStateData( this, widget, duration() ), enabled() ); }
        if( mode&AnimationEnable && !_enableData.contains( widget ) ) { _enableData.insert( widget, new EnableData( this, widget, duration() ), enabled() ); }
        if( mode&AnimationPressed && !_pressedData.contains( widget ) ) { _pressedData.insert( widget, new WidgetStateData( this, widget, duration() ), enabled() ); }

        // connect destruction signal
        connect( widget, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterWidget(QObject*)), Qt::UniqueConnection );

        return true;

    }

    //____________________________________________________________
    BaseEngine::WidgetList WidgetStateEngine::registeredWidgets( AnimationModes mode ) const
    {

        WidgetList out;

        // the typedef is needed to make Krazy happy
        typedef DataMap<WidgetStateData>::Value Value;

        if( mode&AnimationHover )
        {
            foreach( const Value& value, _hoverData )
            { if( value ) out.insert( value.data()->target().data() ); }
        }

        if( mode&AnimationFocus )
        {
            foreach( const Value& value, _focusData )
            { if( value ) out.insert( value.data()->target().data() ); }
        }

        if( mode&AnimationEnable )
        {
            foreach( const Value& value, _enableData )
            { if( value ) out.insert( value.data()->target().data() ); }
        }

        if( mode&AnimationPressed )
        {
            foreach( const Value& value, _pressedData )
            { if( value ) out.insert( value.data()->target().data() ); }
        }

        return out;

    }

    //____________________________________________________________
    bool WidgetStateEngine::updateState( const QObject* object, AnimationMode mode, bool value )
    {
        DataMap<WidgetStateData>::Value data( WidgetStateEngine::data( object, mode ) );
        return ( data && data.data()->updateState( value ) );
    }

    //____________________________________________________________
    bool WidgetStateEngine::isAnimated( const QObject* object, AnimationMode mode )
    {

        DataMap<WidgetStateData>::Value data( WidgetStateEngine::data( object, mode ) );
        return ( data && data.data()->animation() && data.data()->animation().data()->isRunning() );

    }

    //____________________________________________________________
    DataMap<WidgetStateData>::Value WidgetStateEngine::data( const QObject* object, AnimationMode mode )
    {

        switch( mode )
        {
            case AnimationHover: return _hoverData.find( object ).data();
            case AnimationFocus: return _focusData.find( object ).data();
            case AnimationEnable: return _enableData.find( object ).data();
            case AnimationPressed: return _pressedData.find( object ).data();
            default: return DataMap<WidgetStateData>::Value();
        }

    }

}
