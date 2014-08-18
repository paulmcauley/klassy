#ifndef breezedialengine_h
#define breezedialengine_h

//////////////////////////////////////////////////////////////////////////////
// breezedialengine.h
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

#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezedialdata.h"

namespace Breeze
{

    //* stores dial hovered action and timeLine
    class DialEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit DialEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~DialEngine( void )
        {}

        //* register dial
        virtual bool registerWidget( QWidget* );

        //* true if widget is animated
        virtual bool isAnimated( const QObject* object )
        {
            if( DataMap<DialData>::Value data = _data.find( object ) )
            {

                return data.data()->animation().data()->isRunning();

            } else return false;

        }

        //* update state
        virtual bool updateState( const QObject* object, bool state )
        {

            if( DataMap<DialData>::Value data = _data.find( object ) )
            {

                return data.data()->updateState( state );

            } else return false;

        }

        //* animation opacity
        virtual qreal opacity( const QObject* object )
        { return isAnimated( object ) ? _data.find( object ).data()->opacity() : AnimationData::OpacityInvalid; }

        //* control rect
        virtual void setHandleRect( const QObject* object, const QRect& rect )
        {
            if( DataMap<DialData>::Value data = _data.find( object ) )
            { data.data()->setHandleRect( rect ); }
        }

        //* mouse position
        virtual QPoint position( const QObject* object )
        {
            if( DataMap<DialData>::Value data = _data.find( object ) ) return data.data()->position();
            else return QPoint( -1, -1 );
        }

        //* enability
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            /*
            do not disable the map directly, because the contained BreezeScrollbarData
            are also used in non animated mode to store dial handle rect. However
            do disable all contains DATA object, in order to prevent actual animations
            */
            foreach( const DataMap<DialData>::Value data, _data )
            { if( data ) data.data()->setEnabled( value ); }

        }

        //* duration
        virtual void setDuration( int value )
        {
            BaseEngine::setDuration( value );
            _data.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        virtual bool unregisterWidget( QObject* object )
        { return _data.unregisterWidget( object ); }

        private:

        //* data map
        DataMap<DialData> _data;

    };

}

#endif
