//////////////////////////////////////////////////////////////////////////////
// breezebusyindicatorengine.cpp
// handle progress bar busy indicator
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

#include "breezebusyindicatorengine.h"
#include "breezebusyindicatorengine.moc"

#include "breeze.h"

#include <QVariant>

namespace Breeze
{

    //_______________________________________________
    BusyIndicatorEngine::BusyIndicatorEngine( QObject* object ):
        BaseEngine( object ),
        _animation( new Animation( duration(), this ) )
    {

        // setup animation
        _animation.data()->setStartValue( 0 );
        _animation.data()->setEndValue( 2*Metrics::ProgressBar_BusyIndicatorSize );
        _animation.data()->setTargetObject( this );
        _animation.data()->setPropertyName( "value" );
        _animation.data()->setLoopCount( -1 );

    }

    //_______________________________________________
    bool BusyIndicatorEngine::registerWidget( QObject* object )
    {

        // check widget validity
        if( !object ) return false;

         // create new data class
        if( !_data.contains( object ) )
        {
            _data.insert( object, new BusyIndicatorData( this ) );

            // connect destruction signal
            connect( object, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterWidget(QObject*)), Qt::UniqueConnection );
        }

        return true;

    }

    //____________________________________________________________
    bool BusyIndicatorEngine::isAnimated( const QObject* object )
    {

        DataMap<BusyIndicatorData>::Value data( BusyIndicatorEngine::data( object ) );
        return data && data.data()->isAnimated();

    }

    //____________________________________________________________
    void BusyIndicatorEngine::setDuration( int value )
    {

        if( duration() == value ) return;
        BaseEngine::setDuration( value );

        // restart timer with specified time
        _animation.data()->setDuration( value );

    }

    //____________________________________________________________
    void BusyIndicatorEngine::setAnimated( const QObject* object, bool value )
    {

        DataMap<BusyIndicatorData>::Value data( BusyIndicatorEngine::data( object ) );
        if( data )
        {
            // update data
            data.data()->setAnimated( value );

            // start timer if needed
            if( value && !_animation.data()->isRunning() )
            { _animation.data()->start(); }

        }

        return;

    }


    //____________________________________________________________
    DataMap<BusyIndicatorData>::Value BusyIndicatorEngine::data( const QObject* object )
    { return _data.find( object ).data(); }

    //_______________________________________________
    void BusyIndicatorEngine::setValue( int value )
    {

        // update
        _value = value;

        bool animated( false );

        // loop over objects in map
        for( DataMap<BusyIndicatorData>::iterator iter = _data.begin(); iter != _data.end(); ++iter )
        {

            if( iter.value()->isAnimated() )
            {

                // update animation flag
                animated = true;

                // emit update signal on object
                if( const_cast<QObject*>( iter.key() )->inherits( "QQuickStyleItem" ))
                {

                    //QtQuickControls "rerender" method is updateItem
                    QMetaObject::invokeMethod( const_cast<QObject*>( iter.key() ), "updateItem", Qt::QueuedConnection);

                } else {

                    QMetaObject::invokeMethod( const_cast<QObject*>( iter.key() ), "update", Qt::QueuedConnection);

                }

            }

        }

        if( !animated ) _animation.data()->stop();

    }

}
