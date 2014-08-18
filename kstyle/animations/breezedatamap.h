#ifndef breezedatamap_h
#define breezedatamap_h

//////////////////////////////////////////////////////////////////////////////
// breezedatamap.h
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

#include "breeze.h"

#include <QObject>
#include <QMap>
#include <QPaintDevice>

namespace Breeze
{

    //* data map
    /** it maps templatized data object to associated object */
    template< typename K, typename T > class BaseDataMap: public QMap< const K*, WeakPointer<T> >
    {

        public:

        using Key = const K*;
        using Value = WeakPointer<T>;

        //* constructor
        BaseDataMap( void ):
            QMap<Key, Value>(),
            _enabled( true ),
            _lastKey( NULL )
        {}

        //* destructor
        virtual ~BaseDataMap( void )
        {}

        //* insertion
        virtual typename QMap< Key, Value >::iterator insert( const Key& key, const Value& value, bool enabled = true )
        {
            if( value ) value.data()->setEnabled( enabled );
            return QMap< Key, Value >::insert( key, value );
        }

        //* find value
        Value find( Key key )
        {
            if( !( enabled() && key ) ) return Value();
            if( key == _lastKey ) return _lastValue;
            else {
                Value out;
                typename QMap<Key, Value>::iterator iter( QMap<Key, Value>::find( key ) );
                if( iter != QMap<Key, Value>::end() ) out = iter.value();
                _lastKey = key;
                _lastValue = out;
                return out;
            }
        }

        //* unregister widget
        bool unregisterWidget( Key key )
        {

            // check key
            if( !key ) return false;

            // clear last value if needed
            if( key == _lastKey )
            {

                if( _lastValue ) _lastValue.clear();
                _lastKey = NULL;

            }

            // find key in map
            typename QMap<Key, Value>::iterator iter( QMap<Key, Value>::find( key ) );
            if( iter == QMap<Key, Value>::end() ) return false;

            // delete value from map if found
            if( iter.value() ) iter.value().data()->deleteLater();
            QMap<Key, Value>::erase( iter );

            return true;

        }

        //* maxFrame
        void setEnabled( bool enabled )
        {
            _enabled = enabled;
            foreach( const Value& value, *this )
            { if( value ) value.data()->setEnabled( enabled ); }
        }

        //* enability
        bool enabled( void ) const
        { return _enabled; }

        //* duration
        void setDuration( int duration ) const
        {
            foreach( const Value& value, *this )
            { if( value ) value.data()->setDuration( duration ); }
        }

        private:

        //* enability
        bool _enabled;

        //* last key
        Key _lastKey;

        //* last value
        Value _lastValue;

    };

    //* standard data map, using QObject as a key
    template< typename T > class DataMap: public BaseDataMap< QObject, T >
    {

        public:

        //* constructor
        DataMap( void )
        {}

        //* destructor
        virtual ~DataMap( void )
        {}

    };

    //* QPaintDevice based dataMap
    template< typename T > class PaintDeviceDataMap: public BaseDataMap< QPaintDevice, T >
    {

        public:

        //* constructor
        PaintDeviceDataMap( void )
        {}

        //* destructor
        virtual ~PaintDeviceDataMap( void )
        {}

    };

}

#endif
