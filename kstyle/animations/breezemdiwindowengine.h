#ifndef breezemdiwindowengine_h
#define breezemdiwindowengine_h

//////////////////////////////////////////////////////////////////////////////
// breezemdiwindowengine.h
// stores event filters and maps widgets to animations
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
#include "breezemdiwindowdata.h"

namespace Breeze
{

    //* handle mdiwindow arrows hover effect
    class MdiWindowEngine: public BaseEngine
    {

        Q_OBJECT

        public:

        //* constructor
        explicit MdiWindowEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~MdiWindowEngine( void )
        {}

        //* register widget
        virtual bool registerWidget( QWidget* );

        //* state
        virtual bool updateState( const QObject* object, int primitive, bool value )
        {
            if( DataMap<MdiWindowData>::Value data = _data.find( object ) )
            {
                return data.data()->updateState( primitive, value );
            } else return false;
        }

        //* true if widget is animated
        virtual bool isAnimated( const QObject* object, int primitive )
        {
            if( DataMap<MdiWindowData>::Value data = _data.find( object ) )
            {
                return data.data()->isAnimated( primitive );
            } else return false;

        }

        //* animation opacity
        virtual qreal opacity( const QObject* object, int primitive )
        {
            if( DataMap<MdiWindowData>::Value data = _data.find( object ) )
            {
                return data.data()->opacity( primitive );
            } else return AnimationData::OpacityInvalid;
        }

        //* enability
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            _data.setEnabled( value );
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
        DataMap<MdiWindowData> _data;

    };

}

#endif
